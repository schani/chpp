/* -*- c -*- */

/*
 * recorder.h
 *
 * chpp
 *
 * Copyright (C) 1997-1998 Heinz Deinhart
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
  recorder.h
  belief it or not: the recorder records
 ****************************************
*/

#ifndef __recorder_h__
#define __recorder_h__

#define RECORD_BASE_LN_SIZE 1024
#define RECORD_BASE_FN_SIZE 32

/* public: */
const char *unRecordFileName( const int bufPos ); /* returns 0 on error */
int unRecordLineNumber( const int bufPos ); /* returns -1 on wrong bufPos */

/* used in filler.c */
void recordChangeTape();
void recordNewLine( const int bufPos, const int lineNumber );
void recordNewFile( const int bufPos, const char *fileName );

struct RecLN {
  int bufPos[RECORD_BASE_LN_SIZE];
  int lineNr[RECORD_BASE_LN_SIZE];
  struct RecLN *overflow;
};
typedef struct RecLN RecLN;

struct RecFiles {
  int bufPos[RECORD_BASE_FN_SIZE];
  char *fileName[RECORD_BASE_FN_SIZE];
  struct RecFiles *overflow;
};
typedef struct RecFiles RecFiles;

#endif

/* a______b___c_______d */




































/* tutn toetn */
