/* -*- c -*- */

/*
 * commands.h
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

/* Internal commands */

void
commandFatalError( const char *cmd, const char *format, ... );

void
commandWarning( const char *cmd, const char *format, ... );


void cmd_if( const char *what );
void cmd_elseif( const char *what );
void cmd_ifdef( const char *what );
void cmd_ifndef( const char *what );
void cmd_else( const char *arg );
void cmd_endif( const char *arg );
void cmd_discard( const char *arg );
void cmd_endd( const char *arg );
void cmd_include( const char *fileName );
void cmd_edefine( const char *args );
void cmd_ende( const char * args );
void cmd_define( const char *args );
void cmd_error( const char *args );
void cmd_rem( const char *what );
void cmd_end( const char *arg );

/* ------------- Internal defs for flow control */

#define CMD_DISCARD 1

#define CMD_IFDEF 11
#define CMD_IFDEF_ELSE 12
#define CMD_IFNDEF 13
#define CMD_IFNDEF_ELSE 14
#define CMD_IF 15
#define CMD_IF_ELSE 16

#define CMD_EDEF 50


