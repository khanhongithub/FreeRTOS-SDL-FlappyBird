/**
 * @file TUM_Sound.h
 * @author Alex Hoffman
 * @date 27 August 2019
 * @brief A simple interface to play wav files using the SDL2 Mixer library
 *
 * @verbatim
 ----------------------------------------------------------------------
 Copyright (C) Alexander Hoffman, 2019
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------
 @endverbatim
 */

#ifndef __TUM_SOUND_H__
#define __TUM_SOUND_H__

/**
 * @defgroup tum_sound TUM Sound API
 *
 * @brief An API wrapper around the SDL Mixer library to play a set of predefined
 * pong waveforms
 *
 * @{
 */

/**
 * Generates an enum entry for a given string
 */
#define GEN_ENUM(ENUM) ENUM, /*!< ENUM pong sample */

/**
 * A list of wav filenames which are loaded
 */
#define FOR_EACH_SAMPLE(SAMPLE)                                                \
    SAMPLE(a3)                                                             \
    SAMPLE(a4)                                                             \
    SAMPLE(a5)                                                             \
    SAMPLE(b3)                                                             \
    SAMPLE(b4)                                                             \
    SAMPLE(c3)                                                             \
    SAMPLE(c4)                                                             \
    SAMPLE(c5)                                                             \
    SAMPLE(d3)                                                             \
    SAMPLE(d4)                                                             \
    SAMPLE(d5)                                                             \
    SAMPLE(e3)                                                             \
    SAMPLE(e4)                                                             \
    SAMPLE(e5)                                                             \
    SAMPLE(f3)                                                             \
    SAMPLE(f4)                                                             \
    SAMPLE(f5)                                                             \
    SAMPLE(g3)                                                             \
    SAMPLE(g4)                                                             \
    SAMPLE(g5)

/**
 * @enum tumSound_samples_e
 * @brief Enum containing the currently loaded wav samples
 *
 * The waveforms located in the resource/waveforms folder are added using the
 * @ref FOR_EACH_SAMPLE macro to generate an enum that can be used to access
 * the appropriate sample from the internal sample list.
 */
enum tumSound_samples_e { FOR_EACH_SAMPLE(GEN_ENUM) };

/**
 * @brief Initializes the SDL2 Mixer library and loads the wav samples specified
 * in the @ref tumSound_samples_e
 *
 * @param bin_dir_str String specifying where the program's binary is located
 * @return 0 on success
 */
int tumSoundInit(char *bin_dir_str);

/**
 * @brief Deinitializes the SDL2 Mixer library
 */
void tumSoundExit(void);

/**
 * @brief Plays a wav sample
 *
 * @param index Index to specify which sample to play, @ref tumSound_samples_e gives
 * appropriate indices
 * @return NULL always returns NULL
 */
void tumSoundPlaySample(unsigned char index);

/** @}*/

#endif
