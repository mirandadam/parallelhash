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

#ifndef _HASHER_SHA256_H_
#define _HASHER_SHA256_H_

#include "hasher.h"
#include "lib/sha256.h"

class Hasher_SHA256: public Hasher
    {

    public:
        Hasher_SHA256();
        virtual ~Hasher_SHA256();

    protected:
        virtual void Update_State(const uint8_t* data, uint64_t data_size);
        virtual void Calculate_Hash();
        virtual void Reset_Hash_State();

        sha256_ctx state;

    private:
    };

#endif // _HASHER_SHA256_H_
