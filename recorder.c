/* -*- c -*- */

/*
 * recorder.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"

#include "recorder.h"

static int didInit = 0;

// line number stuff
static RecLN *recLN, *activeRecLN;
static int recLNPos;

// file name stuff
static RecFiles *recFiles, *activeRecFiles;
static int recFilesPos;

const char *unRecordFileName( const int bufPos ) {
  RecFiles *tmpRecFiles = recFiles;
  int i;
  const char *str = 0;

  while( tmpRecFiles != activeRecFiles ) {
    str = tmpRecFiles->fileName[0];
    for( i=0; i<(RECORD_BASE_LN_SIZE-1); i++ ) {
      if( tmpRecFiles->bufPos[i] > bufPos ) return str;
      str = tmpRecFiles->fileName[i];
    }
    tmpRecFiles = tmpRecFiles->overflow;
  }
  // last buffer
  str = tmpRecFiles->fileName[0];
  for(  i=0; i<(recFilesPos-1); i++ ) {
    if( tmpRecFiles->bufPos[i] > bufPos ) return str;
    str = tmpRecFiles->fileName[i];;
  }

  return str;
}

int unRecordLineNumber( const int bufPos ) {
  RecLN *tmpRecLN = recLN;
  int i, lNr = -1;

  while( tmpRecLN != activeRecLN ) {
     
      lNr = tmpRecLN->lineNr[0];
    for( i=0; i<(RECORD_BASE_LN_SIZE-1); i++ ) {
      if( tmpRecLN->bufPos[i] > bufPos ) return lNr;
      lNr = tmpRecLN->lineNr[i];
    }
    tmpRecLN = tmpRecLN->overflow;
  }
  /* last buffer */
  lNr = tmpRecLN->lineNr[0];
  for( i=0; i<(recLNPos-1); i++ ) {
    if( tmpRecLN->bufPos[i] > bufPos ) return lNr;
    lNr = tmpRecLN->lineNr[i];
  }

  return lNr;
}

void recordDoInit() {
  recLN = (RecLN *)memXAlloc( sizeof( RecLN ));
  recLN->overflow = NULL;
  recFiles = (RecFiles *)memXAlloc( sizeof( RecFiles ));
  recFiles->overflow = NULL;
}

void recordChangeTape() {
  if( !didInit ) recordDoInit();
  
  while( recLN->overflow ) { // Kill all overflows
    RecLN *tmpRecLN = recLN->overflow;
    recLN->overflow = tmpRecLN->overflow;
    memFree( tmpRecLN );
  }
  recLNPos = 0; // reset counta
  activeRecLN = recLN; // hide overflows

  while( recFiles->overflow ) { // Kill all overflows
    RecFiles *tmpRecFiles = recFiles->overflow;
    recFiles->overflow = tmpRecFiles->overflow;
    memFree( tmpRecFiles );
  }
  recFilesPos = 0; // reset counta
  activeRecFiles = recFiles; // hide overflows
}

void recordNewLine( const int bufPos, const int lineNumber ) {
  if( recLNPos > RECORD_BASE_LN_SIZE ) {
    fprintf( stderr, "\nFATAL ERROR in recorder.c: recLNPos is illegal, call the police!\n" );
    exit( 1 );
  }
  if( recLNPos == RECORD_BASE_LN_SIZE ) { // uh ah .. overflow
    activeRecLN->overflow = (RecLN *)memXAlloc( sizeof( RecLN ));
    recLNPos = 0;
    activeRecLN = activeRecLN->overflow;
  }

  activeRecLN->lineNr[recLNPos] = lineNumber;
  activeRecLN->bufPos[recLNPos++] = bufPos;
}

void recordNewFile( const int bufPos, const char *fileName ) {
  if( recFilesPos > RECORD_BASE_FN_SIZE ) {
    fprintf( stderr, "\nFATAL ERROR in recorder.c: recFilesPos is illegal, call the police!\n" );
    exit( 1 );
  }
  if( recFilesPos == RECORD_BASE_FN_SIZE ) { // uh ah .. overflow
    fprintf( stderr, "-------------------------------\n" );
    activeRecFiles->overflow = (RecFiles *)memXAlloc( sizeof( RecFiles ));
    recFilesPos = 0;
    activeRecFiles = activeRecFiles->overflow;
  }

  activeRecFiles->bufPos[recFilesPos] = bufPos;
  activeRecFiles->fileName[recFilesPos++] =
      strcpy((char*)memXAllocAtomic(strlen(fileName) + 1), fileName);
}
