/* -*- c -*- */

/*
 * filler.c
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

/*****************************************************************************
 *  Command parse ,,black box'' (named by Schani)
 *
 * Created by a fluctuation of void *
 */

#define _FILLER_C_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "memory.h"
#include "dynstring.h"
#include "chash.h"
#include "error.h"
#include "depends.h"

#include "filler.h"
#include "commands.h"
#include "recorder.h"

/* #undef MERD */
/* define MERD(x) ; */

/* include paths */
dynstring *defDirs = 0;
int nrOfDefDirs = 0;

/* used by filler & commands */
int bufPosAtCommandBegin;

char filler_cmdChar;
int flowsExtern = 0;
dynstring externFlow;

FlowStack *flowStack;
int flowDepth = 1;

int incFiles;
int zeilenNummer = 0;
dynstring currentFileName;
dynstring currentFilePath;

InputFileStack *ifStack = 0;
/* ------------------------- */

static FILE* inFile;
static int activeNL, bufOverflow;
static dynstring tmpStr, overflowStr;
/* static int commandWantsFlush = 0; */
static dynstring nextCmdCommand;
static dynstring nextCmdArgument;
static void *nextCmdFun = 0;

static hash cmdHash;

int warningAlert = 0;

void
cutExtern( int i ) {
  dsShrinkRear( &externFlow, i );
}

void
goExtern( char ch ) {
  dsAppendChar( &externFlow, ch );
}

void
goExternDS( dynstring *ds ) {
  dsAppendString( &externFlow, ds->data, ds->length );
}

void
flowDepthCeck() {
  if( flowDepth == 1 ) {
    fprintf( stderr, 
	    "chmop: flowDepth should be greater than 1, but is 1.\n" );
   exit( 1 );
  }
}

dynstring getPathFromFileName( const char *name ) {
  char *tmp = memXAllocAtomic( strlen( name ) + 1 );
  char *p;
  dynstring ds;

  strcpy( tmp, name );
  p = strrchr( tmp, '/' );
  if( !p ) return dsNewFrom( "" );
  *(p+1)=0;

  ds = dsNewFrom( tmp );
  memFree( tmp );
  
  return ds;
}

dynstring getLastPathComponent( const char *path ) {
  char *p;
  int i = strlen( path );

  if( !i || path[i-1] == '/' ) return dsNewFrom( "" );
  p = strrchr( path, '/' );
  return dsNewFrom( p ? (p+1) : path );
}

/* returns 0 if file not found */
int
includeFile( const char *name ) {
  InputFileStack *tmpIFS;
  FILE *ifi = 0;
  dynstring tmpDy, path;
  int strlen_name = strlen( name );


  path.data = 0;

  /* filenames starting with "./" are always local */
  if( nrOfDefDirs && name[0] != '.' && name[1] != '/' ) { 
    int i;

    for( i = (nrOfDefDirs-1); i>=0; i-- ) {
      tmpDy = dsCopy( &defDirs[i] );
      dsAppendString( &tmpDy, name, strlen_name );
      if( (ifi = fopen( tmpDy.data, "r" )) != 0 ) {
	i = 0;
	path = getPathFromFileName( tmpDy.data );
      }
    }
  }
  
  if( !ifi ) { /* try to open file relativ to old file */
    tmpDy = dsCopy( &currentFilePath );
    dsAppendString( &tmpDy, name, strlen_name );
    if( !(ifi = fopen( tmpDy.data, "r" ))) { /* try working dir */
      if( !(ifi = fopen( name, "r" ))) {
	return 0;
      } else {
	path = dsNewFrom( "./" );
	//	return 1;  *** warum steht das hier ?
      }
    } else {
      path = getPathFromFileName( tmpDy.data );
    }
  }

  if( !ifi ) { /* file not found */
    // issueError(ERRCMD_NO_INCLUDE, name, 0, 0);
    return 0;
  }

  tmpIFS = ifStack;
  ifStack = (InputFileStack *)memXAlloc(sizeof(InputFileStack));
  ifStack->next = tmpIFS;
  ifStack->f = inFile;
  ifStack->zeilenNummer = zeilenNummer;
  ifStack->name = currentFileName;
  ifStack->path = currentFilePath;
  currentFileName = getLastPathComponent( name );

  currentFilePath = path;
  zeilenNummer = 0;
  inFile = ifi;
  activeNL = 1;

  recordNewFile( bufPosAtCommandBegin, name );

  if( currentFilePath.length >= 2 && !strncmp( currentFilePath.data, "./",2 ))
    dsShrinkFront( &currentFilePath, 2 );
  
  incFiles++;

  if( generateDependencies ) {
    dynstring tmpDynni = dsNewFrom( currentFilePath.data );
    dsAppendString( &tmpDynni, currentFileName.data,
                    strlen( currentFileName.data ));
    dsAppendChar( &tmpDynni, ' ' );
    addDependency( 0, &tmpDynni );
  }

  return 1;
}

void
finishFile() {
  InputFileStack *tmpIFS ;

  if( ifStack ) { /* not with last file */
    fclose( inFile );
    tmpIFS = ifStack;
    inFile = ifStack->f;
    zeilenNummer = ifStack->zeilenNummer;
    /*
    fprintf( stderr, "i wipe di aus du sau (%p).\n",
             currentFilePath.data );
             */
    currentFileName = ifStack->name;
    currentFilePath = ifStack->path;
    ifStack = tmpIFS->next;
    recordNewFile( bufPosAtCommandBegin, currentFileName.data );
  }
  incFiles--;
}

static void
generateCommands() {
  cmdHash = hash_new( 100 );

  hash_insert( cmdHash, "include", cmd_include );
  
  hash_insert( cmdHash, "define", cmd_define );
  hash_insert( cmdHash, "def", cmd_define );
  hash_insert( cmdHash, "ende", cmd_ende );

  hash_insert( cmdHash, "edefine", cmd_edefine );
  hash_insert( cmdHash, "edef", cmd_edefine );

  hash_insert( cmdHash, "discard", cmd_discard );
  hash_insert( cmdHash, "disc", cmd_discard );
  hash_insert( cmdHash, "endd", cmd_endd );

  hash_insert( cmdHash, "ifdefined", cmd_ifdef );
  hash_insert( cmdHash, "ifdef", cmd_ifdef );
  hash_insert( cmdHash, "ifnotdefined", cmd_ifndef );
  hash_insert( cmdHash, "ifndef", cmd_ifndef );
  hash_insert( cmdHash, "if", cmd_if );
  hash_insert( cmdHash, "else", cmd_else );
  hash_insert( cmdHash, "endif", cmd_endif );
  hash_insert( cmdHash, "elseif", cmd_elseif );
  hash_insert( cmdHash, "elif", cmd_elseif );

  hash_insert( cmdHash, "end", cmd_end );
  
  hash_insert( cmdHash, "!", cmd_rem );
  hash_insert( cmdHash, "rem", cmd_rem );
  hash_insert( cmdHash, "error", cmd_error );
}

void
initCommands( FILE *inputFile, char commandChar, const char *filename, const char *filepath) {
  inFile = inputFile;
  incFiles = 1;
  filler_cmdChar = commandChar;
  activeNL = 1;
  tmpStr = dsNew();
  ifStack = 0;
  bufOverflow = 0;
  generateCommands();

  /* init flowStack */
  flowStack = (FlowStack *)memXAlloc( sizeof( FlowStack ));
  flowStack->next = 0;
  flowStack->flowLevel = 1;
  flowStack->openCmd = 0;

  currentFileName = dsNewFrom(filename);
  currentFilePath = dsNewFrom(filepath);
}

/* processActiveNL
 * Starts to copy a line to tmp buffer till
 * 1) it knows that line is no command -> tmp buf is copied to output buf
 *     and processText is called
 * 2) it knows that line is a command .. command is execd
 */
int
processActiveNL( char *buf, int max, int akt ) {
  int bufp;
  char ch;

  zeilenNummer++;

  recordNewLine( akt, zeilenNummer );
 
  if( !tmpStr.data ) tmpStr = dsNew();

  /* MERD( "processActiveNL" ); */
  bufp = akt;
  ch = (char)fgetc( inFile );
  /*fprintf( stderr, "-%c-", ch ); */
  /*  fprintf( stderr, "<<processActiveNL - %i - %i - ch = '%c'>>\n", max, akt, ch ); */
  /*fprintf( stderr, "-%c-", ch ); */
  while( (ch == ' ' || ch == '\t') && !feof( inFile )) {
      /*   fprintf( stderr, "kurding '%c'\n", ch ); */
    dsAppendChar( &tmpStr, ch );
    ch = (char)fgetc( inFile );
    /* fprintf( stderr, "-%c-", ch ); */
  }

  if( feof( inFile )) {
    ch = 0;
  }

  if( ch ) {
    if( flowStack->flowLevel ) { /* HACK, but i weiss net warum der char */
	/* durchrutscht wenn i des net mach .... */
      dsAppendChar( &tmpStr, ch );
    }/* dont miss the ch who killed command */
    /*    fprintf( stderr, "kurding II '%c'\n", ch ); */
  }

  if( ch == filler_cmdChar ) { /* oh yes, it is a command */
    nextCmdCommand = dsNew();
    nextCmdArgument = dsNew();

    do { /* read rest of line */
      ch = (char)fgetc( inFile );
      dsAppendChar( &nextCmdCommand, ch );
    } while( ch != '\n' && !feof( inFile));

    dsShrinkRear( &nextCmdCommand, 1 );
    dsRemoveOuterWS( &nextCmdCommand );
    nextCmdArgument = dsSplitWS( &nextCmdCommand );

    dsRemoveOuterWS( &nextCmdArgument );

    /*    fprintf( stderr, "[COMMAND FOUND: '%s' '%s']\n",  */
    /*	    nextCmdCommand.data, nextCmdArgument.data ); */

    {
      nextCmdFun = hash_lookup( cmdHash, nextCmdCommand.data );
      if( !nextCmdFun ) {
	issueWarning(WARNCMD_UNKNOWN_COMMAND, nextCmdCommand.data, 0, 0);
      } else {
	  /*	((void(*)(const char*))fun)( nextCmdArgument.data ); */
	return bufp; /* return now, next time go to processCommand */

      }
    }

    tmpStr = dsNew();
    
    /* aktiveNL stays 1 cause command may follow in next line */
  } else { /* no command, pass tmpStr to schani */
      /*MERD( "reingfalln, is doch kein command" ); */
    if( flowsExtern ) { /* in externen buffa eine */
      goExternDS( &tmpStr );
      /*      fprintf( stderr, "activeNL: '%s'\n", tmpStr.data ); */
      if( ch != '\n' ) {
	activeNL = 0;
      } else {
	tmpStr = dsNew();
      }
    } else {

      if( !flowStack->flowLevel || tmpStr.length <= (max-akt-1)) { 
	  /* super, buffer gross genug, oder eh kein output */
	if( flowStack->flowLevel ) {
	  memcpy( &buf[bufp], tmpStr.data, tmpStr.length );
	  bufp += tmpStr.length;
	}
	
	if( ch != '\n' ) {
	  activeNL = 0;
	} else {
	  tmpStr = dsNew();
	}
      } else { /* mega scheisse, buffer to short */
	  /* fprintf( stderr, "*#*#*#*#*#*#*#*#*quake\n" ); */
	memcpy( &buf[bufp], tmpStr.data, max - akt - 1 ); /* fully fill buffer */
	bufp += max - akt - 1;
	/* store rest of tmpStr in overflowStr */
	overflowStr = dsCopy( &tmpStr );
	dsShrinkFront( &overflowStr, max - akt - 1 );
	bufOverflow = 1;
      }
    }
  }
  
  if( feof( inFile )) { /* Change files */
      /* go down 1 include level */
      bufPosAtCommandBegin = bufp; /* hack: end of file is no command */
    finishFile();
    activeNL = 1;
    return bufp;
  }

  return bufp;
}

/* BUG: may loose a character if buffer is full */

/* processText
 * reads chars, and wipes them into buffer
 * if newline is encounterd, processActiveNL is called
 */
int
processText( char *buf, int max, int akt ) {
  int bufp;
  char ch;

  /* fprintf( stderr, "<<processText - %i - %i ->>\n", max, akt ); */

  bufp = akt;

  ch = (char)fgetc( inFile );
  /* fprintf( stderr, "-%c-", ch ); */
  while( ch != '\n' && bufp < (max - 1) && !feof( inFile )) {
      /* add chars */
    if( flowStack->flowLevel ) {  /* if flowLevel write to buf */
      if( flowsExtern ) {
	goExtern( ch );
	/*	fprintf( stderr, "processText: '%c'\n", ch ); */
      } else {
        buf[bufp++] = ch;
      }
    }
    ch = (char)fgetc( inFile );
    /* fprintf( stderr, "-%c-", ch ); */
  }

  if( feof( inFile )) {
    finishFile();
    return bufp;
  } else {
    if( ch == '\n' && bufp < max) { /* add newline if possible */
       if( flowStack->flowLevel ) {
	 if( flowsExtern ) {
	   goExtern( ch );
	   /*   fprintf( stderr, "processText: '%c'\n", ch ); */
	 } else {
	   buf[bufp++] = ch;
	 }
       }
    }
      
    if( bufp == (max - 1)) {
      overflowStr = dsNew();
      if( ch != '\n' ) dsAppendChar( &overflowStr, ch );
      /* fprintf( stderr, "+> %i\n", unRecordLineNumber( bufp )); */
      bufOverflow = 1;
    }
  }

  if( ch == '\n' ) {
    activeNL = 1;
    tmpStr = dsNew();
    return bufp;
  }
  return bufp;
}

/* processOverflow
 * 1) handles the last char that did not fit into the output buffer
 * 2) splits the buffer created by processActiveNL into output buffer
 */
int
processOverflow( char *buf, int max, int akt) {
  int bufp = akt;
  char ch;

  /*  fprintf( stderr, "PROCESS OVERFLOW\n" );*/

  if( overflowStr.length <= (max-akt-1)) {  /* super, buffer gross genug */
    memcpy( &buf[bufp], overflowStr.data, overflowStr.length );
    bufp += overflowStr.length;
    
    ch = buf[bufp-1]; /* needed to check if last char was newline */

    if( ch != '\n' ) {
      activeNL = 0;
    } else {
      activeNL = 1;
      overflowStr = dsNew();
    }
    bufOverflow = 0;
  } else { /* mega scheisse, buffer to short */
    memcpy( &buf[bufp], overflowStr.data, max - akt - 1 ); /* fully fill buffer */
    /* store rest of tmpStr in overflowStr */
    dsShrinkFront( &overflowStr, max - akt - 1 );
    /* lets do it again */
  }
  
  return bufp;
}

int
processCommand( char *buf, int max, int akt ) {

    /* Hack to access the bufPos inside of commands (used in include) */
  bufPosAtCommandBegin = akt;
  
  if( nextCmdArgument.data[0] ) {
    ((void(*)(const char*))nextCmdFun)( nextCmdArgument.data );
  } else {
    ((void(*)(const char*))nextCmdFun)( 0 );
  }
  tmpStr = dsNew();

  nextCmdFun = 0;

  /*  fprintf( stderr, "processCommand: tmpStr = '%s'\n", tmpStr.data ); */

  activeNL = 1;

  return akt;
}

int /* returnd anzahl or -1 if all done */
fillBuffer( char *buf, int max ) { /* max is anzahl */
    /* scan 4 cmd */
  int bufp = 0;

  recordChangeTape();
  recordNewFile( 0, currentFileName.data );
  
  if( !incFiles ) return -1;
  if( nextCmdFun ) bufp = processCommand( buf, max, bufp );
  do {
    if( bufOverflow ) bufp = processOverflow( buf, max, bufp );
    if( activeNL ) { /* tmpStr muss allociert sein */
      bufp = processActiveNL( buf, max, bufp );
      if( nextCmdFun ) return bufp; /* aha, command wants flush */
    } else {
      bufp = processText( buf, max, bufp );
    }
  } while( bufp < (max - 1) && incFiles );
  return bufp;
}

/*
int
fillBuffer( char *buf, int max ) {
  int xxx;

  xxx= fillBuffer2( buf, max );
  fprintf( stderr, "-> %i (bufoverflow=%i activeNL=%i)\n",
	   unRecordLineNumber( xxx ), bufOverflow, activeNL );
  return xxx;
}
*/
