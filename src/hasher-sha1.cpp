/***************************************************************************
 *   Copyright (C) 2009 by Daniel Araujo Miranda                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2, as    *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include "hasher-sha1.h"

Hasher_SHA1::Hasher_SHA1()
    {
    hash_algorithm=sha1;
    hash_size_bytes=20;
    }

Hasher_SHA1::~Hasher_SHA1()
    {
    }

void Hasher_SHA1::Update_Full_State(const uint8_t* data, uint64_t data_size)
    {
    sha1_process_bytes(data, data_size, &full_state);
    }

void Hasher_SHA1::Update_Window_State(const uint8_t* data, uint64_t data_size)
    {
    sha1_process_bytes(data, data_size, &window_state);
    }

void Hasher_SHA1::Calculate_Full_Hash()
    {
    sha1_finish_ctx(&full_state, full_result);
    }

void Hasher_SHA1::Calculate_Window_Hash()
    {
    sha1_finish_ctx(&window_state, window_result);
    }

void Hasher_SHA1::Reset_Full_Hash_State()
    {
    sha1_init_ctx(&full_state);
    }

void Hasher_SHA1::Reset_Window_Hash_State()
    {
    sha1_init_ctx(&window_state);
    }