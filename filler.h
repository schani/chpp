/* -*- c -*- */

/*
 * filler.h
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

#include <stdio.h>
#include "dynstring.h"

/* am anfang aufrufen */
void
initCommands( FILE *inputFile, char commandChar, const char *filename, const char *filepath );

int /* returnd anzahl or -1 if all done */
fillBuffer( char *buf, int max ); /* max is anzahl */

struct InputFileStack {
  FILE *f;
  int zeilenNummer;
  dynstring name;
  struct InputFileStack *next;
  dynstring path;
};
typedef struct InputFileStack InputFileStack;

struct FlowStack {
  int flowLevel;
  int openCmd;
  struct FlowStack *next;
};
typedef struct FlowStack FlowStack;


/* ALERT ALERT SCHANI ALERT SCHANI SCHANI */
/* des zeug auf extern aendern, und du musst des providen */

extern dynstring *defDirs;
extern int nrOfDefDirs;
extern int generateDependencies;

#define MERD(muell) fprintf( stderr, "MERD: %s\n", muell );

/* protos for commands.c */
void flowDepthCeck();
int includeFile( const char *name );
void finishFile();
