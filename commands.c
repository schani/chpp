/* -*- c -*- */

/*
 * commands.c
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

#include "commands.h"
#include "filler.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "memory.h"
#include "dynstring.h"
/*#include "macros.h"*/
/*#include "symtab.h"*/
#include "arith.h"
#include "error.h"
#include "parser.h"
#include "input.h"

extern int bufPosAtCommandBegin;

extern char filler_cmdChar;
extern int flowsExtern;
extern dynstring externFlow;
extern FlowStack *flowStack;
extern int flowDepth;
extern int zeilenNummer;
extern int incFiles;
extern dynstring currentFileName;
extern InputFileStack *ifStack;

/* static flowLevel = 1; */

static dynstring evalDS( dynstring *ds ) {
  dynstring eds = dsNew();
  inputReader ir = irNewDynstring(ds, 0);
  outputWriter ow = owNewDynstring(&eds);
  
  parParseUntil(&ir, 0, bcwNewOutput(globalEnvironment, &ow), 1, globalEnvironment, 0);
  return eds;
}

static dynstring eatDSUntil( dynstring *body, int *pos, const char *set ) {
  int origPos = *pos;
  while( (*pos) < body->length ) {
    const char *setElement = set;

    while( (*setElement) != 0 ) {
      if( (*setElement) == body->data[(*pos)] ) {
	return dsNewFromBytes( &(body->data[origPos]), (*pos) - origPos );
      }
      setElement++;
    }
    (*pos)++;
  }
  return dsCopy( body );
}

void
emmitLocation( const char *muh ) {
  dynstring fHira;
  InputFileStack *ifs = ifStack;
  static char kurdenBuffer[100];

  fHira = dsNew();
  if( incFiles > 1 ) {
    dsAppendString( &fHira, " (", 2 );
    while( ifs ) {
      dsAppendString( &fHira, ifs->name.data, ifs->name.length );
      dsAppendChar( &fHira, ':' );
      sprintf( kurdenBuffer, "%i", ifs->zeilenNummer );
      dsAppendString( &fHira, kurdenBuffer, strlen( kurdenBuffer ));
      ifs = ifs->next;
      if( ifs ) dsAppendString( &fHira, ", ", 2 );
    }
    dsAppendChar( &fHira, ')' );
  }
  fprintf( stderr, "%s: file %s:%i%s\n",
	   muh, currentFileName.data, zeilenNummer, fHira.data );
}

/*
void
commandFatalError( const char *cmd, const char *format, ... ) {
  va_list ap;

  emmitLocation( "ERROR" );
  
  fprintf( stderr, "       %c%s: ", filler_cmdChar, cmd );
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
  fprintf( stderr, "\n" );
  exit( 1 );
}

void
commandWarning( const char *cmd, const char *format, ... ) {
  va_list ap;

  emmitLocation( "WARNING" );
  
  fprintf( stderr, "         %c%s: ", filler_cmdChar, cmd );
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
  fprintf( stderr, "\n" );
}
*/

void
cmd_error( const char *what ) {
  issueError( ERRCMD_USER_ERROR, what, 0, 0 );
}

void
cmd_rem( const char *what ) {
  // 1 + 1 = 3
}

void
cmd_elseif( const char *what ) {
  cmd_endif( 0 );
  cmd_if( what );
}

void
cmd_if( const char *what ) {
  FlowStack *tmpFS = (FlowStack *)memXAlloc( sizeof( FlowStack ));
  dynstring eds, ds = dsNewFrom( what );

  if( flowStack->flowLevel ) {
    dynstring tmpDS;
    dsRemoveOuterWS( &ds );
    eds = evalDS( &ds );
    
    tmpDS = arithEvalDS( &eds, globalEnvironment );
    if( atoi( tmpDS.data )) tmpFS->flowLevel = 1;
    else tmpFS->flowLevel = 0; /* discard all */
  } else {
    tmpFS->flowLevel = 0;
  }
  tmpFS->openCmd = CMD_IF;
  
  tmpFS->next = flowStack;
  flowStack = tmpFS;
  flowDepth++;
}

void
cmd_ifdef( const char *what ) {
  FlowStack *tmpFS = (FlowStack *)memXAlloc( sizeof( FlowStack ));
  dynstring eds, ds = dsNewFrom( what );

  if( flowStack->flowLevel ) {
    dsRemoveOuterWS( &ds );
    eds = evalDS( &ds );
    
    if( envGetBinding(globalEnvironment, &eds ) != 0 ) tmpFS->flowLevel = 1;
    else tmpFS->flowLevel = 0; /* discard all */
  } else {
    tmpFS->flowLevel = 0;
  }
  tmpFS->openCmd = CMD_IFDEF;
  
  tmpFS->next = flowStack;
  flowStack = tmpFS;
  flowDepth++;
}

void
cmd_ifndef( const char *what ) {
  FlowStack *tmpFS = (FlowStack *)memXAlloc( sizeof( FlowStack ));
  dynstring eds, ds = dsNewFrom( what );

  if( flowStack->flowLevel ) {
    dsRemoveOuterWS( &ds );
    eds = evalDS( &ds );
    
    if( envGetBinding(globalEnvironment, &eds ) == 0) tmpFS->flowLevel = 1;
    else tmpFS->flowLevel = 0; /* discard all */
  } else {
    tmpFS->flowLevel = 0;
  }
  tmpFS->openCmd = CMD_IFNDEF;
  
  tmpFS->next = flowStack;
  flowStack = tmpFS;
  flowDepth++;
}

void
cmd_else( const char *arg ) {
   if( arg ) issueWarning(WARNCMD_UNUSED_ARGS, "else", 0, 0);

   switch( flowStack->openCmd ) {
   case CMD_IFDEF:
   case CMD_IFNDEF:
   case CMD_IF:
     flowStack->openCmd++; /* IFXX_ELSE is always IFXX + 1 !! */
     flowStack->flowLevel = (flowStack->flowLevel + 1) % 2;

     /* wenn ma schon im discard war, hilfts nix */
     if( flowStack->next && !flowStack->next->flowLevel )
       flowStack->flowLevel = 0;
     break;
   default:
       issueError(ERRCMD_UNMATCHED_COMMAND, "else", 0, 0);
   }
}

void
cmd_endif( const char *arg ) {
  FlowStack *tmpFS = flowStack;

  if( arg ) issueWarning(WARNCMD_UNUSED_ARGS, "endif", 0, 0);

  switch( flowStack->openCmd ) {
  case CMD_IFDEF:
  case CMD_IFDEF_ELSE:
  case CMD_IFNDEF:
  case CMD_IFNDEF_ELSE:
  case CMD_IF:
  case CMD_IF_ELSE:
    flowDepthCeck();
    flowDepth--;
    flowStack = tmpFS->next;
    memFree( tmpFS );
    break;
  default:
    issueError( ERRCMD_UNMATCHED_COMMAND, "endif", 0, 0 );
  }
}

void
cmd_discard( const char *arg ) {
  FlowStack *tmpFS = (FlowStack *)memXAlloc( sizeof( FlowStack ));

  if( arg ) issueWarning( WARNCMD_UNUSED_ARGS, "discard", 0, 0);

  tmpFS->flowLevel = 0; /* discard all */
  tmpFS->openCmd = CMD_DISCARD;
  
  tmpFS->next = flowStack;
  flowStack = tmpFS;
  flowDepth++;
}

void
cmd_endd( const char *arg ) {
  FlowStack *tmpFS = flowStack;

  if( arg ) issueWarning( WARNCMD_UNUSED_ARGS, "endd", 0, 0);
  if( flowStack->openCmd != CMD_DISCARD )
    issueError( ERRCMD_UNMATCHED_COMMAND, "endd", 0, 0);

  flowDepthCeck();
  flowDepth--;
  flowStack = tmpFS->next;
  memFree( tmpFS );
}

void
cmd_include( const char *fileName ) {
  dynstring evald, fn = dsNewFrom( fileName );
/*  FILE *ifi;*/

  if( !flowStack->flowLevel ) return;
  
  evald = evalDS( &fn );
  dsRemoveOuterWS( &evald );
/* // obsolete:
  
  ifi = fopen( evald.data, "r" );
  if( !ifi )
    issueError(ERRCMD_NO_INCLUDE, evald.data, 0, 0);
  includeFile( ifi, fileName );
  */

  if( !includeFile( evald.data ))
    issueError(ERRCMD_NO_INCLUDE, evald.data, 0, 0);
  else {
   
  }
}

static dynstring *edef_dynAr;
static int edef_dynNr;
static dynstring edef_evaldName;
static int edef_itIsSimple;

void
cmd_edefine( const char *args ) {
  dynstring name, body = dsNewFrom( args );
  int pos = 0;
  FlowStack *tmpFS = (FlowStack *)memXAlloc( sizeof( FlowStack ));

  if( flowStack->flowLevel ) {
    tmpFS->flowLevel = 1; /* discard all */
  } else {
    tmpFS->flowLevel = 0;
  }
  tmpFS->openCmd = CMD_EDEF;
  
  tmpFS->next = flowStack;
  flowStack = tmpFS;
  flowDepth++;

  if( !flowStack->flowLevel ) return;

  dsRemoveOuterWS( &body ); /* only 1 arg, so we can remove the spaces */

  name = eatDSUntil( &body, &pos, " \t(" );
  if( body.data[pos] == '(' ) { /* macro with arguments */
    dynstring parms = eatDSUntil( &body, &pos, ")" );

    if( pos == body.length )
      issueError(ERRCMD_CLOSE_PAREN_EXPECTED, 0, 0, 0);
    if( pos != body.length - 1 ) {
      issueError(ERRCMD_ONE_ARG, "edefine", 0, 0);
    }
    /* dsShrinkFront( &body, pos+1 ); */
    dsRemoveOuterWS( &body );
    if( parms.length ) { /* parse arguments outya ! */
      dynstring first = dsNew();
      dynstring second = dsNew();
      int pos = 1;
      int dynSize = 8;

      edef_dynAr = (dynstring *)memXAlloc( sizeof( dynstring ) * 8 );
      edef_dynNr = 0;
      second = eatDSUntil( &parms, &pos, "," );
      dsRemoveOuterWS( &second );
      evalDS( &second );
      edef_dynAr[edef_dynNr++] = dsCopy( &second );
      pos++;
      while( parms.length > pos ) {
	first = eatDSUntil( &parms, &pos, "," );
	dsRemoveOuterWS( &first );
	evalDS( &first );
	pos++;
	if( edef_dynNr == dynSize ) {
	  dynSize += 8;
	  edef_dynAr = (dynstring *)memXRealloc( edef_dynAr, sizeof( dynstring ) * 
						 dynSize );
	}
	edef_dynAr[edef_dynNr++] = dsCopy( &first );
      }
      
      edef_itIsSimple = 0;
      edef_evaldName = evalDS( &name );
      flowsExtern = 1;
      externFlow = dsNew();
      return;
      /*     defineGlobalUserDefined( &evaldName, edef_dynNr, dynAr, &body ); */
    }
    /* else no */
  } /* macro with arguments */

  if( pos != body.length )
    issueError(ERRCMD_ONE_ARG, "edefine", 0, 0);

  edef_itIsSimple = 1;
  edef_evaldName = evalDS( &name );
  /*
//  dsShrinkFront( &body, pos+1 );
//  dsRemoveOuterWS( &body );
//  edef_body = dsCopy( &body );
//  defineGlobalVariable( &evaldName, &body );
*/
  flowsExtern = 1;
  externFlow = dsNew();
}

void
cmd_ende( const char * args ) {
  FlowStack *tmpFS = flowStack;
  dynstring ef2;

  if( args ) issueWarning(WARNCMD_UNUSED_ARGS, "endd", 0, 0);
  if( flowStack->openCmd != CMD_EDEF )
    issueError(ERRCMD_UNMATCHED_COMMAND, "ende", 0, 0);

  flowDepthCeck();
  flowDepth--;
  flowStack = tmpFS->next;
  memFree( tmpFS );

  flowsExtern = 0;

  dsShrinkRear( &externFlow, 1 );
  ef2 = evalDS( &externFlow );

  /* now define the macro */
  if( edef_itIsSimple ) {
    envModifyOrAddBinding(globalEnvironment, &edef_evaldName,
			  valueNewScalar(&ef2), globalEnvironment);
  } else {
    inputReader ir = irNewDynstring(&ef2, 0);
    
    envModifyOrAddBinding(globalEnvironment, &edef_evaldName,
			  valueNewLambda(edef_dynNr, 0, 0, edef_dynAr, '%',
					 parParseIntoBCUntil(&ir, 0, 1,
							     globalEnvironment, 0),
					 globalEnvironment),
			  globalEnvironment);

  }
}

void
cmd_define( const char *args ) {
  dynstring evaldName, name, body = dsNewFrom( args );
  int pos = 0;

  if( !flowStack->flowLevel ) return;

  name = eatDSUntil( &body, &pos, " \t(" );
  if( body.data[pos] == '(' ) { /* macro with arguments */
    dynstring parms = eatDSUntil( &body, &pos, ")" );

    if( pos == body.length )
      issueError(ERRCMD_CLOSE_PAREN_EXPECTED, 0, 0, 0);
    dsShrinkFront( &body, pos+1 );
    dsRemoveOuterWS( &body );
    if( parms.length ) { /* parse arguments outya ! */
      dynstring first = dsNew();
      dynstring second = dsNew();
      int pos = 1;
      dynstring *dynAr = (dynstring *)memXAlloc( sizeof( dynstring ) * 8 );
      int dynNr = 0;
      int dynSize = 8;

      second = eatDSUntil( &parms, &pos, "," );
      dsRemoveOuterWS( &second );
      evalDS( &second );
      dynAr[dynNr++] = dsCopy( &second );
      pos++;
      while( parms.length > pos ) {
	first = eatDSUntil( &parms, &pos, "," );
	dsRemoveOuterWS( &first );
	evalDS( &first );
	pos++;
	if( dynNr == dynSize ) {
	  dynSize += 8;
	  dynAr = (dynstring *)memXRealloc( dynAr, sizeof( dynstring ) * dynSize );
	}
	dynAr[dynNr++] = dsCopy( &first );
      }
      
      evaldName = evalDS( &name );
      {
	inputReader ir = irNewDynstring(&evaldName, 0);

      envModifyOrAddBinding(globalEnvironment, &evaldName,
			    valueNewLambda(dynNr, 0, 0, dynAr, '%',
					   parParseIntoBCUntil(&ir, 0, 1,
							       globalEnvironment, 0),
					   globalEnvironment),
			    globalEnvironment);
      }
      return;
    }
    /* else no */
  } /* macro with arguments */

  evaldName = evalDS( &name );
  dsShrinkFront( &body, pos+1 );
  dsRemoveOuterWS( &body );    
  envModifyOrAddBinding(globalEnvironment, &evaldName,
			valueNewScalar(&body), globalEnvironment);
}
