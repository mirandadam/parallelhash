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

#include "hasher-sha512.h"

Hasher_SHA512::Hasher_SHA512()
    {
    hash_algorithm=sha512;
    hash_size_bytes=64;
    }

Hasher_SHA512::~Hasher_SHA512()
    {
    }

void Hasher_SHA512::Update_State(const uint8_t* data, uint64_t data_size)
    {
    sha512_process_bytes(data, data_size, &state);
    }

void Hasher_SHA512::Calculate_Hash()
    {
    sha512_finish_ctx(&state, result);
    }

void Hasher_SHA512::Reset_Hash_State()
    {
    sha512_init_ctx(&state);
    }
