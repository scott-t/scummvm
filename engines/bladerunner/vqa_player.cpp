/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/vqa_player.h"

#include "bladerunner/bladerunner.h"

#include "audio/decoders/raw.h"

#include "common/system.h"

namespace BladeRunner {

bool VQAPlayer::open(const Common::String &name) {
	_s = _vm->getResourceStream(name);
	if (!_s) {
		return false;
	}

	if (!_decoder.loadStream(_s)) {
		delete _s;
		_s = nullptr;
		return false;
	}

	_hasAudio = _decoder.hasAudio();
	if (_hasAudio) {
		_audioStream = Audio::makeQueuingAudioStream(_decoder.frequency(), false);
	}

	_repeatsCount = 0;
	_loop = -1;
	_frame = -1;
	_frameBegin = -1;
	_frameEnd = _decoder.numFrames() - 1;
	_frameEndQueued = -1;
	_repeatsCountQueued = -1;

	if (_loopInitial >= 0) {
		setLoop(_loopInitial, _repeatsCountInitial, kLoopSetModeImmediate, nullptr, nullptr);
	} else {
		_frameNext = 0;
		setBeginAndEndFrame(0, _frameEnd, 0, kLoopSetModeJustStart, nullptr, nullptr);
	}

	return true;
}

void VQAPlayer::close() {
	_vm->_mixer->stopHandle(_soundHandle);
	delete _s;
	_s = nullptr;
}

int VQAPlayer::update(bool forceDraw, bool advanceFrame, Graphics::Surface *customSurface) {
	uint32 now = 60 * _vm->_system->getMillis();
	int result = -1;

	if (_frameNext < 0) {
		_frameNext = _frameBegin;
	}

	if ((_repeatsCount > 0 || _repeatsCount == -1) && (_frameNext > _frameEnd)) {
		int loopEndQueued = _frameEndQueued;
		if (_frameEndQueued != -1) {
			_frameEnd = _frameEndQueued;
			_frameEndQueued = -1;
		}
		if (_frameNext != _frameBegin) {
			_frameNext = _frameBegin;
		}

		if (loopEndQueued == -1) {
			if (_repeatsCount != -1) {
				_repeatsCount--;
			}
			//callback for repeat, it is not used in the blade runner
		} else {
			_repeatsCount = _repeatsCountQueued;
			_repeatsCountQueued = -1;

			if (_callbackLoopEnded != nullptr) {
				_callbackLoopEnded(_callbackData, 0, _loop);
			}
		}

		result = -1;
	} else if (_frameNext > _frameEnd) {
		result = -3;
	} else if (now < _frameNextTime) {
		result = -1;
	} else if (advanceFrame) {
		_frame = _frameNext;
		_decoder.readFrame(_frameNext, kVQAReadVideo);
		_decoder.decodeVideoFrame(customSurface != nullptr ? customSurface : _surface, _frameNext);

		int audioPreloadFrames = 14;

		if (_hasAudio) {
			if (!_audioStarted) {
				for (int i = 0; i < audioPreloadFrames; i++) {
					if (_frameNext + i < _frameEnd) {
						_decoder.readFrame(_frameNext + i, kVQAReadAudio);
						queueAudioFrame(_decoder.decodeAudioFrame());
					}
				}
				_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _audioStream);
				_audioStarted = true;
			}
			if (_frameNext + audioPreloadFrames < _frameEnd) {
				_decoder.readFrame(_frameNext + audioPreloadFrames, kVQAReadAudio);
				queueAudioFrame(_decoder.decodeAudioFrame());
			}
		}
		if (_frameNextTime == 0) {
			_frameNextTime = now + 60000 / 15;
		}
		else {
			_frameNextTime += 60000 / 15;
		}
		_frameNext++;
		result = _frame;
	}

	if (result < 0 && forceDraw && _frame != -1) {
		_decoder.decodeVideoFrame(customSurface != nullptr ? customSurface : _surface, _frame, true);
		result = _frame;
	}
	return result;
}

void VQAPlayer::updateZBuffer(ZBuffer *zbuffer) {
	_decoder.decodeZBuffer(zbuffer);
}

void VQAPlayer::updateView(View *view) {
	_decoder.decodeView(view);
}

void VQAPlayer::updateScreenEffects(ScreenEffects *screenEffects) {
	_decoder.decodeScreenEffects(screenEffects);
}

void VQAPlayer::updateLights(Lights *lights) {
	_decoder.decodeLights(lights);
}

bool VQAPlayer::setLoop(int loop, int repeatsCount, int loopSetMode, void (*callback)(void *, int, int), void *callbackData) {
#if BLADERUNNER_DEBUG_CONSOLE
	debug("VQAPlayer::setBeginAndEndFrameFromLoop(%i, %i, %i), streamLoaded = %i", loop, repeatsCount, loopSetMode, _s != nullptr);
#endif
	if (_s == nullptr) {
		_loopInitial = loop;
		_repeatsCountInitial = repeatsCount;
		return true;
	}

	int begin, end;
	if (!_decoder.getLoopBeginAndEndFrame(loop, &begin, &end)) {
		return false;
	}
	if (setBeginAndEndFrame(begin, end, repeatsCount, loopSetMode, callback, callbackData)) {
		_loop = loop;
		return true;
	}
	return false;
}

bool VQAPlayer::setBeginAndEndFrame(int begin, int end, int repeatsCount, int loopSetMode, void (*callback)(void *, int, int), void *callbackData) {
#if BLADERUNNER_DEBUG_CONSOLE
	debug("VQAPlayer::setBeginAndEndFrame(%i, %i, %i, %i), streamLoaded = %i", begin, end, repeatsCount, loopSetMode, _s != nullptr);
#endif

	if (repeatsCount < 0) {
		repeatsCount = -1;
	}

	if (_repeatsCount == 0 && loopSetMode == kLoopSetModeEnqueue) {
		loopSetMode = kLoopSetModeImmediate;
	}

	//TODO: there is code in original game which deals with changing loop at start of loop, is it nescesarry? loc_46EA04

	_frameBegin = begin;

	if (loopSetMode == kLoopSetModeJustStart) {
		_repeatsCount = repeatsCount;
	} else if (loopSetMode == kLoopSetModeEnqueue) {
		_repeatsCountQueued = repeatsCount;
		_frameEndQueued = end;
	} else if (loopSetMode == kLoopSetModeImmediate) {
		_repeatsCount = repeatsCount;
		_frameEnd = end;
		seekToFrame(begin);
	}

	_callbackLoopEnded = callback;
	_callbackData = callbackData;

	return true;
}

bool VQAPlayer::seekToFrame(int frame) {
	_frameNext = frame;
	_frameNextTime = 60 * _vm->_system->getMillis();
	return true;
}

int VQAPlayer::getLoopBeginFrame(int loop) {
	int begin, end;
	if (!_decoder.getLoopBeginAndEndFrame(loop, &begin, &end)) {
		return -1;
	}
	return begin;
}

int VQAPlayer::getLoopEndFrame(int loop) {
	int begin, end;
	if (!_decoder.getLoopBeginAndEndFrame(loop, &begin, &end)) {
		return -1;
	}
	return end;
}

int VQAPlayer::getFrameCount() {
	return _decoder.numFrames();
}

void VQAPlayer::queueAudioFrame(Audio::AudioStream *audioStream) {
	int n = _audioStream->numQueuedStreams();
	if (n == 0)
		warning("numQueuedStreams: %d", n);
	_audioStream->queueAudioStream(audioStream, DisposeAfterUse::YES);
}

} // End of namespace BladeRunner
