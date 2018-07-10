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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "startrek/room.h"

#define OBJECT_GLOB 8
#define OBJECT_SPLIT_GLOB_1 9
#define OBJECT_SPLIT_GLOB_2 10
#define OBJECT_11 11

#define HOTSPOT_INSIGNIA 0x20 // This doesn't seem to be visible anywhere
#define HOTSPOT_WALL 0x21     // Same here
#define HOTSPOT_DOOR 0x22


#define GLOB_X 0x46
#define GLOB_Y 0xaf

#define SPLIT_GLOB_1_X 0x20
#define SPLIT_GLOB_1_Y 0xaf

#define SPLIT_GLOB_2_X 0x69
#define SPLIT_GLOB_2_Y 0xaf

namespace StarTrek {

// This room has a good deal of unused code, relating to the "split" globs (automatons).
// Apparently, it would originally have been possible to interact with the split globs,
// instead of having them kill the crew right away. This doesn't seem to have quite been
// finished, though.

extern const RoomAction trial2ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0},  &Room::trial2Tick1 },
	{ {ACTION_TICK, 60, 0, 0}, &Room::trial2Tick60 },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0},     &Room::trial2TouchedHotspot0 },
	{ {ACTION_LOOK, OBJECT_KIRK,      0, 0}, &Room::trial2LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,     0, 0}, &Room::trial2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,     0, 0}, &Room::trial2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,  0, 0}, &Room::trial2LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_INSIGNIA, 0, 0}, &Room::trial2LookAtInsignia },
	{ {ACTION_LOOK, OBJECT_GLOB,         0, 0}, &Room::trial2LookAtGlob },
	{ {ACTION_LOOK, OBJECT_SPLIT_GLOB_1, 0, 0}, &Room::trial2LookAtGlob },
	{ {ACTION_LOOK, OBJECT_SPLIT_GLOB_2, 0, 0}, &Room::trial2LookAtGlob },
	{ {ACTION_LOOK, HOTSPOT_WALL, 0, 0}, &Room::trial2LookAtWall },
	{ {ACTION_LOOK, HOTSPOT_DOOR, 0, 0}, &Room::trial2LookAtDoor },

	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::trial2TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::trial2TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::trial2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::trial2TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_GLOB,         0, 0}, &Room::trial2TalkToGlob },
	{ {ACTION_TALK, OBJECT_SPLIT_GLOB_1, 0, 0}, &Room::trial2TalkToGlob },
	{ {ACTION_TALK, OBJECT_SPLIT_GLOB_2, 0, 0}, &Room::trial2TalkToGlob },

	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_WALL, 0}, &Room::trial2UsePhaserOnWall },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_WALL, 0}, &Room::trial2UsePhaserOnWall },

	{ {ACTION_DONE_WALK, 7,  0, 0},                  &Room::trial2ReachedPositionToShootGlob },
	{ {ACTION_DONE_ANIM, 10, 0, 0},                  &Room::trial2DrewPhaserToShootGlob },
	{ {ACTION_DONE_ANIM, 4, 0, 0},                   &Room::trial2GlobDoneExploding },
	{ {ACTION_DONE_ANIM, 1, 0, 0},                   &Room::trial2GlobDoneSplitting },
	{ {ACTION_DONE_ANIM, 19, 0, 0},                  &Room::trial2KirkDied },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_GLOB, 0}, &Room::trial2UseStunPhaserOnGlob },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_GLOB, 0}, &Room::trial2UseKillPhaserOnGlob },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseStunPhaserOnSplitGlob1 },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseKillPhaserOnSplitGlob1 },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseStunPhaserOnSplitGlob2 },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseKillPhaserOnSplitGlob2 },

	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,         0}, &Room::trial2UseMTricorderOnKirk },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,        0}, &Room::trial2UseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,        0}, &Room::trial2UseMTricorderOnMccoy },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT,     0}, &Room::trial2UseMTricorderOnRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_GLOB,         0}, &Room::trial2UseMTricorderOnGlob },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseMTricorderOnGlob },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseMTricorderOnGlob },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_WALL,        0}, &Room::trial2UseSTricorderOnWall },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,                0}, &Room::trial2UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_GLOB,         0}, &Room::trial2UseSTricorderOnGlob },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseSTricorderOnGlob },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseSTricorderOnGlob },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff,                   0}, &Room::trial2UseCommunicator },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_SPLIT_GLOB_1,    0}, &Room::trial2UseMccoyOnGlob },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_SPLIT_GLOB_2,    0}, &Room::trial2UseMccoyOnGlob },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_WALL,           0}, &Room::trial2UseMccoyOnWall },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_SPLIT_GLOB_1,    0}, &Room::trial2UseSpockOnGlob },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_SPLIT_GLOB_2,    0}, &Room::trial2UseSpockOnGlob },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_WALL,           0}, &Room::trial2UseSpockOnWall },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseRedshirtOnGlob },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseRedshirtOnGlob },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_WALL,        0}, &Room::trial2UseRedshirtOnWall },

	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0}, &Room::trial2WalkToDoor },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0}, &Room::trial2UseMedkitAnywhere },

	// ENHANCEMENT: Define these actions for the main glob, not just the (unused) split
	// globs
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_GLOB, 0}, &Room::trial2UseMccoyOnGlob },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_GLOB, 0}, &Room::trial2UseSpockOnGlob },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_GLOB, 0}, &Room::trial2UseRedshirtOnGlob },
};

extern const int trial2NumActions = sizeof(trial2ActionList) / sizeof(RoomAction);


void Room::trial2Tick1() {
	playVoc("NOOOLOOP");

	if (!_vm->_awayMission.trial.enteredGlobRoom)
		_vm->_awayMission.disableInput = 2;

	if (!_vm->_awayMission.trial.globDefeated) {
		playMidiMusicTracks(MIDITRACK_24, -1);
		loadMapFile("trial22");

		if (!_vm->_awayMission.trial.globSplitInTwo) {
			playVoc("TRI2LOOP");
			loadActorAnim2(OBJECT_GLOB, "sglob", 0x46, 0xaf);
			_vm->_awayMission.trial.globEnergyLevels[0] = 1;
		} else {
			playVoc("TRI2LOOP");
			loadActorAnim2(OBJECT_SPLIT_GLOB_1, "sglob", 0x20, 0xaf);
			loadActorAnim2(OBJECT_SPLIT_GLOB_2, "sglob", 0x69, 0xaf);
			_vm->_awayMission.trial.globEnergyLevels[1] = 1;
			_vm->_awayMission.trial.globEnergyLevels[2] = 1;
		}
	}
}

void Room::trial2Tick60() {
	if (!_vm->_awayMission.trial.enteredGlobRoom) {
		_vm->_awayMission.disableInput = false;
		showText(TX_SPEAKER_BENNIE, TX_TRI2_034);
		showText(TX_SPEAKER_KIRK, TX_TRI2_005);
		showText(TX_SPEAKER_SPOCK, TX_TRI2_030);
		showText(TX_SPEAKER_KIRK, TX_TRI2_008);
		_vm->_awayMission.trial.enteredGlobRoom = true;
	}
}

void Room::trial2TouchedHotspot0() { // This is unused
	if (_vm->_awayMission.trial.globEnergyLevels[1] != 0 || _vm->_awayMission.trial.globEnergyLevels[2] != 0)
		showText(TX_SPEAKER_SPOCK, TX_TRI2_024);
}

void Room::trial2LookAtKirk() {
	showText(TX_TRI2N003);
}

void Room::trial2LookAtSpock() {
	showText(TX_TRI2N001);
}

void Room::trial2LookAtMccoy() {
	showText(TX_TRI2N006);
}

void Room::trial2LookAtRedshirt() {
	showText(TX_TRI2N002);
}

void Room::trial2LookAtInsignia() {
	showText(TX_TRI2N004);
}

void Room::trial2LookAtGlob() {
	showText(TX_TRI2N000);
}

void Room::trial2LookAtWall() {
	showText(TX_TRI2N007);
}

void Room::trial2LookAtDoor() {
	showText(TX_TRI2N005);
}

void Room::trial2TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_TRI2_001);
}

void Room::trial2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_TRI2_019);
}

void Room::trial2TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_TRI2_013);
}

void Room::trial2TalkToRedshirt() {
	showText(TX_SPEAKER_BENNIE, TX_TRI2_033);
}

void Room::trial2TalkToGlob() {
	showText(TX_QUIET);
}

void Room::trial2UsePhaserOnWall() {
	showText(TX_TRI2N008);
}


void Room::trial2UsePhaserOnGlob(int object, bool phaserOnKill) {
	_roomVar.trial.globBeingShot = object;
	_roomVar.trial.phaserOnKill = phaserOnKill;

	_vm->_awayMission.disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xcd, 0xc5, &Room::trial2ReachedPositionToShootGlob);
}

void Room::trial2ReachedPositionToShootGlob() {
	loadActorAnimC(OBJECT_KIRK, "kdraww", -1, -1, &Room::trial2DrewPhaserToShootGlob);
}

void Room::trial2DrewPhaserToShootGlob() {
	const char *stunPhaserAnims[] = {
		"t2kp00",
		"t2kp01",
		"t2kp02",
	};
	const char *killPhaserAnims[] = {
		"t2kp03",
		"t2kp04",
		"t2kp05",
	};

	int index = _roomVar.trial.globBeingShot - OBJECT_GLOB;

	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	playSoundEffectIndex(SND_PHASSHOT);
	if (_roomVar.trial.phaserOnKill)
		showBitmapFor5Ticks(killPhaserAnims[index], 5);
	else
		showBitmapFor5Ticks(stunPhaserAnims[index], 5);
	loadActorStandAnim(OBJECT_KIRK);

	_vm->_awayMission.disableInput = false;

	if (_roomVar.trial.phaserOnKill)
		_vm->_awayMission.trial.globEnergyLevels[index] += 2;
	else
		_vm->_awayMission.trial.globEnergyLevels[index] += 1;

	if (!(_roomVar.trial.globBeingShot == OBJECT_GLOB && _roomVar.trial.phaserOnKill == true)) {
		if (_roomVar.trial.phaserOnKill)
			showText(TX_SPEAKER_SPOCK, TX_TRI2_023);
		else
			showText(TX_SPEAKER_SPOCK, TX_TRI2_025);
	}


	const Common::Point globPositions[] = {
		Common::Point(GLOB_X, GLOB_Y),
		Common::Point(SPLIT_GLOB_1_X, SPLIT_GLOB_1_Y),
		Common::Point(SPLIT_GLOB_2_X, SPLIT_GLOB_2_Y)
	};

	if (_vm->_awayMission.trial.globEnergyLevels[index] == 3) {
		playVoc("GLOBEDIV");
		loadActorAnimC(_roomVar.trial.globBeingShot, "globsp", globPositions[index].x, globPositions[index].y, &Room::trial2GlobDoneSplitting);
	} else if (_vm->_awayMission.trial.globEnergyLevels[index] >= 4) {
		playSoundEffectIndex(SND_BLANK_14);
		playVoc("REDBALL");
		loadActorAnimC(_roomVar.trial.globBeingShot, "globex", globPositions[index].x, globPositions[index].y, &Room::trial2GlobDoneExploding);
	}
}

void Room::trial2GlobDoneExploding() {
	stopAllVocSounds();
	playVoc("Noooloop");
	showText(TX_SPEAKER_SPOCK, TX_TRI2_020);
	_vm->_awayMission.trial.globDefeated = true;
	playMidiMusicTracks(MIDITRACK_28, -1);
	_vm->_awayMission.trial.missionScore += 1;
	loadMapFile("trial2");
}

void Room::trial2GlobDoneSplitting() {
	if (_roomVar.trial.globBeingShot == OBJECT_GLOB) {
		if (true)
			showText(TX_SPEAKER_SPOCK, TX_TRI2_009);
		else {
			// Unused code block: instead of killing the crew right away, the crew can
			// interact with the split globs.
			_vm->_awayMission.trial.globEnergyLevels[0] = 0;
			_vm->_awayMission.trial.globEnergyLevels[1] = 1;
			_vm->_awayMission.trial.globEnergyLevels[2] = 1;
			_vm->_awayMission.trial.globSplitInTwo = true;
			loadActorAnim2(OBJECT_SPLIT_GLOB_1, "sglob", SPLIT_GLOB_1_X, SPLIT_GLOB_1_Y);
			loadActorAnim2(OBJECT_SPLIT_GLOB_2, "sglob", SPLIT_GLOB_2_X, SPLIT_GLOB_2_Y);
			loadActorStandAnim(OBJECT_GLOB);
			showText(TX_SPEAKER_SPOCK, TX_TRI2_021);
			return;
		}
	} else if (_roomVar.trial.globBeingShot == OBJECT_SPLIT_GLOB_1) {
		loadActorAnim2(OBJECT_11, "sglob", 0x43, 0xaf);
		loadActorAnim2(OBJECT_SPLIT_GLOB_1, "sglob", 0, 0xaf);
	} else if (_roomVar.trial.globBeingShot == OBJECT_SPLIT_GLOB_2) {
		loadActorAnim2(OBJECT_11, "sglob", 0x8c, 0xaf);
		loadActorAnim2(OBJECT_SPLIT_GLOB_2, "sglob", 0x46, 0xaf);
	}

	// Everyone gets vaporized
	playVoc("V7ALLGET");
	_vm->_awayMission.disableInput = true;
	playMidiMusicTracks(MIDITRACK_26, -1);
	loadActorAnimC(OBJECT_KIRK, "kkills", -1, -1, &Room::trial2KirkDied);
	loadActorAnim2(OBJECT_SPOCK, "skills");
	loadActorAnim2(OBJECT_MCCOY, "mkills");
	loadActorAnim2(OBJECT_REDSHIRT, "rkills");
}

void Room::trial2KirkDied() {
	_vm->_awayMission.disableInput = false;
	showGameOverMenu();
}

void Room::trial2UseStunPhaserOnGlob() {
	trial2UsePhaserOnGlob(OBJECT_GLOB, false);
}

void Room::trial2UseKillPhaserOnGlob() {
	trial2UsePhaserOnGlob(OBJECT_GLOB, true);
}

void Room::trial2UseStunPhaserOnSplitGlob1() {
	trial2UsePhaserOnGlob(OBJECT_SPLIT_GLOB_1, false);
}

void Room::trial2UseKillPhaserOnSplitGlob1() {
	trial2UsePhaserOnGlob(OBJECT_SPLIT_GLOB_1, true);
}

void Room::trial2UseStunPhaserOnSplitGlob2() {
	trial2UsePhaserOnGlob(OBJECT_SPLIT_GLOB_2, false);
}

void Room::trial2UseKillPhaserOnSplitGlob2() {
	trial2UsePhaserOnGlob(OBJECT_SPLIT_GLOB_2, true);
}

void Room::trial2UseMTricorderOnKirk() {
	mccoyScan(DIR_S, TX_TRI2_016, true);
}

void Room::trial2UseMTricorderOnSpock() {
	mccoyScan(DIR_S, TX_TRI2_017, true);
}

void Room::trial2UseMTricorderOnMccoy() {
	mccoyScan(DIR_S, TX_TRI2_014, true);
}

void Room::trial2UseMTricorderOnRedshirt() {
	mccoyScan(DIR_S, TX_TRI2_015, true);
}

void Room::trial2UseMTricorderOnGlob() {
	mccoyScan(DIR_S, TX_TRI2_011, true);

	if (!_vm->_awayMission.trial.gotPointsForScanningGlob) {
		_vm->_awayMission.trial.gotPointsForScanningGlob = true;
		_vm->_awayMission.trial.missionScore += 1;
	}
}

void Room::trial2UseSTricorderOnWall() {
	spockScan(DIR_S, TX_TRI2_029, true);
}

void Room::trial2UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_TRI2_027, true);
}

void Room::trial2UseSTricorderOnGlob() {
	spockScan(DIR_S, TX_TRI2_026, true);

	if (!_vm->_awayMission.trial.gotPointsForScanningGlob) {
		_vm->_awayMission.trial.gotPointsForScanningGlob = true;
		_vm->_awayMission.trial.missionScore += 1;
	}
}

void Room::trial2UseCommunicator() {
	if (_vm->_awayMission.trial.forceFieldDown) { // TODO: Refactor this between rooms?
		showText(TX_SPEAKER_UHURA, TX_TRI2U091);

		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_TRI2_003, TX_TRI2_004, TX_TRI2_007,
			TX_BLANK
		};
		int choice = showText(choices);

		if (choice == 0) { // "Beam us back to the enterprise"
			_vm->_awayMission.trial.field5f = 1;
			endMission(_vm->_awayMission.trial.missionScore, 1, 1); // FIXME: Inconsistent with TRIAL1
		} else if (choice == 1) { // "Beam us to Vlict's position"
			showText(TX_SPEAKER_UHURA, TX_TRI1U080); // NOTE: Original didn't show text here
			_vm->_awayMission.disableInput = true;
			loadRoomIndex(4, 4);
		} // Else don't transport anywhere
	} else { // Force field still up
		showText(TX_SPEAKER_UHURA, TX_TRI2U087);
		showText(TX_SPEAKER_KIRK,  TX_TRI2_006);
		showText(TX_SPEAKER_UHURA, TX_TRI2U104);
		if (!_vm->_awayMission.trial.globDefeated) {
			showText(TX_SPEAKER_UHURA, TX_TRI2U081);
			showText(TX_SPEAKER_KIRK,  TX_TRI2_002);
		}
		showText(TX_SPEAKER_UHURA, TX_TRI2U074);
	}
}

void Room::trial2UseMccoyOnGlob() {
	showText(TX_SPEAKER_MCCOY, TX_TRI2_012);
}

void Room::trial2UseMccoyOnWall() {
	showText(TX_SPEAKER_MCCOY, TX_TRI2_010);
}

void Room::trial2UseSpockOnGlob() {
	// NOTE: Two possible audio files to use, TRI2_028 and TRI2_F11
	showText(TX_SPEAKER_SPOCK, TX_TRI2_028);
}

void Room::trial2UseSpockOnWall() {
	showText(TX_SPEAKER_SPOCK, TX_TRI2_022);
}

void Room::trial2UseRedshirtOnGlob() {
	showText(TX_SPEAKER_BENNIE, TX_TRI2_031);
}

void Room::trial2UseRedshirtOnWall() {
	showText(TX_SPEAKER_BENNIE, TX_TRI2_032);
}

void Room::trial2WalkToDoor() {
	walkCrewman(OBJECT_KIRK, 0x117, 0xb5);
}

void Room::trial2UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, TX_TRI2_018);
}

}