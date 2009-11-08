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

#include "hasher-sha256.h"

Hasher_SHA256::Hasher_SHA256()
    {
    hash_algorithm=sha256;
    hash_size_bytes=32;
    }

Hasher_SHA256::~Hasher_SHA256()
    {
    }

void Hasher_SHA256::Update_Full_State(const uint8_t* data, uint64_t data_size)
    {
    sha256_process_bytes(data, data_size, &full_state);
    }

void Hasher_SHA256::Update_Window_State(const uint8_t* data, uint64_t data_size)
    {
    sha256_process_bytes(data, data_size, &window_state);
    }

void Hasher_SHA256::Calculate_Full_Hash()
    {
    sha256_finish_ctx(&full_state, full_result);
    }

void Hasher_SHA256::Calculate_Window_Hash()
    {
    sha256_finish_ctx(&window_state, window_result);
    }

void Hasher_SHA256::Reset_Full_Hash_State()
    {
    sha256_init_ctx(&full_state);
    }

void Hasher_SHA256::Reset_Window_Hash_State()
    {
    sha256_init_ctx(&window_state);
    }