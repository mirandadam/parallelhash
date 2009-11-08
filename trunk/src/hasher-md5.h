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

#ifndef _HASHER_MD5_H_
#define _HASHER_MD5_H_

#include "hasher.h"
#include "lib/md5.h"

class Hasher_MD5: public Hasher
    {

    public:
        Hasher_MD5();
        virtual ~Hasher_MD5();

    protected:
        virtual void Update_Full_State(const uint8_t* data, uint64_t data_size);
        virtual void Update_Window_State(const uint8_t* data, uint64_t data_size);
        virtual void Calculate_Full_Hash();
        virtual void Calculate_Window_Hash();
        virtual void Reset_Full_Hash_State();
        virtual void Reset_Window_Hash_State();

        md5_ctx full_state;
        md5_ctx window_state;

    private:

    };

#endif // _HASHER_MD5_H_
