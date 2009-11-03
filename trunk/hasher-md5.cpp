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

#include "hasher-md5.h"

Hasher_MD5::Hasher_MD5()
    {
    hash_algorithm=md5;
    hash_size_bytes=16;
    }

Hasher_MD5::~Hasher_MD5()
    {
    }

void Hasher_MD5::Update_State(const uint8_t* data, uint64_t data_size)
    {
    md5_process_bytes(data, data_size, &state);
    }

void Hasher_MD5::Calculate_Hash()
    {
    md5_finish_ctx(&state, result);
    }

void Hasher_MD5::Reset_Hash_State()
    {
    md5_init_ctx(&state);
    }
