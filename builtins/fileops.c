/* -*- c -*- */

/*
 * builtins/fileops.c
 *
 * chpp
 *
 * Copyright (C) 1997-1998 Mark Probst
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#ifdef _NEXT
#include <libc.h>
#endif

#include "../memory.h"
#include "../error.h"
#include "../environment.h"
#include "../output.h"

#include "builtins.h"

static FILE **fileTable = 0;
static int fileTableSize = 0;
static int numOpenFiles = 0;

#define INITIAL_FILE_TABLE_SIZE    8
#define FILE_TABLE_GROW_AMOUNT     8

static int
getNewFileNum (void)
{
    int fileNum;

    if (fileTable == 0)
    {
	int i;

	fileTableSize = INITIAL_FILE_TABLE_SIZE;
	fileTable = (FILE**)memXAlloc(fileTableSize * sizeof(FILE*));
	for (i = 0; i < fileTableSize; ++i)
	    fileTable[i] = 0;
	fileNum = 0;
    }
    else if (numOpenFiles < fileTableSize)
    {
	for (fileNum = 0; fileTable[fileNum] != 0; ++fileNum)
	    ;
    }
    else
    {
	int i;

	fileNum = fileTableSize;
	fileTableSize += FILE_TABLE_GROW_AMOUNT;
	fileTable = (FILE**)memXRealloc(fileTable, fileTableSize * sizeof(FILE*));
	for (i = fileNum; i < fileTableSize; ++i)
	    fileTable[i] = 0;
    }

    return fileNum;
}

void
builtInFopen (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int fileNum;
    char fileNumString[64],
	*openMode = "r";

    if (!(numArgs == 1 || numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "fopen");
	return;
    }

    if (numArgs == 2)
    {
	transformArgumentToScalar(&args[1]);
	openMode = args[1].value.value->v.scalar.scalar.data;
    }

    if (strcmp(openMode, "r") != 0 && strcmp(openMode, "w") != 0 && strcmp(openMode, "a") != 0 )
    {
	OUT_STRING(ow, "-1", 2);
	return;
    }

    fileNum = getNewFileNum();
    fileTable[fileNum] = fopen(transformArgumentToScalar(&args[0])->v.scalar.scalar.data,
			       openMode);
    if (fileTable[fileNum] == 0)
    {
	OUT_STRING(ow, "-1", 2);
	return;
    }

    ++numOpenFiles;
    OUT_STRING(ow, fileNumString, sprintf(fileNumString, "%d", fileNum));
}

void
builtInFpipe (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int thePipe[2];
    int fileNum;
    char fileNumString[64];
    enum { PIPE_READ, PIPE_WRITE } pipeMode = PIPE_READ;

    if (!(numArgs >= 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "fpipe");
	return;
    }

    if (pipe(thePipe) == -1)
    {
	issueError(ERRMAC_CALL_FAILED, "fpipe", strerror(errno), 0);
	OUT_STRING(ow, "-1", 2);
    }

    transformArgumentToScalar(&args[0]);
    
    if (args[0].value.value->v.scalar.scalar.length > 1 ||
	(args[0].value.value->v.scalar.scalar.length == 1 && 
	 strcmp(args[0].value.value->v.scalar.scalar.data, "r") != 0 &&
	 strcmp(args[0].value.value->v.scalar.scalar.data, "w") != 0) )
    {
	issueError(ERRMAC_INVALID_MACRO_ARG, "fpipe", args[0].value.value->v.scalar.scalar.data, 0);
	OUT_STRING(ow, "-1", 2);
    }

    if (args[0].value.value->v.scalar.scalar.length == 1 &&
	strcmp(args[0].value.value->v.scalar.scalar.data, "w") == 0)
        pipeMode = PIPE_WRITE;

    switch (fork())
    {
	case -1 :
	    issueError(ERRMAC_CALL_FAILED, "fork", strerror(errno), 0);
	    OUT_STRING(ow, "-1", 2);

	case 0 :
	    {
		int i;
		char **execArgs = (char**)memXAlloc((numArgs + 1) * sizeof(char*));

		switch (pipeMode)
		{
		  case PIPE_READ:
		    close(thePipe[0]);
		    dup2(thePipe[1], 1);
		    break;

		  default:
		    close(thePipe[1]);
		    dup2(thePipe[0], 0);
		}

		for (i = 1; i < numArgs; ++i)
		    execArgs[i-1] = transformArgumentToScalar(&args[i])->v.scalar.scalar.data;
		execArgs[i-1] = 0;

		if (execvp(execArgs[0], execArgs) == -1)
		{
		    issueError(ERRMAC_CALL_FAILED, "execvp", strerror(errno), 0);
		    exit(1);
		}
	    }
	    break;

	default :
	    fileNum = getNewFileNum();

	    switch (pipeMode)
	    {
	      case PIPE_READ:
		close(thePipe[1]);
		fileTable[fileNum] = fdopen(thePipe[0], "r");
		break;

	      default:
		close(thePipe[0]);
		fileTable[fileNum] = fdopen(thePipe[1], "w");
	    }

	    if (fileTable[fileNum] == 0)
	    {
		issueError(ERRMAC_CALL_FAILED, "fdopen", strerror(errno), 0);
		OUT_STRING(ow, "-1", 2);
	    }

	    break;
    }

    ++numOpenFiles;
    OUT_STRING(ow, fileNumString, sprintf(fileNumString, "%d", fileNum));
}

void
builtInFclose (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int fileNum;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "fclose");
	return;
    }

    fileNum = atoi(transformArgumentToScalar(&args[0])->v.scalar.scalar.data);

    if (fileNum < 0 || fileNum >= fileTableSize || fileTable[fileNum] == 0)
	issueError(ERRMAC_INVALID_MACRO_ARG, "fclose", args[0].value.value->v.scalar.scalar.data, 0);
    else
    {
	fclose(fileTable[fileNum]);
	fileTable[fileNum] = 0;
	--numOpenFiles;
    }
}

void
builtInFgets (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    FILE *file;
    int fileNum,
	result;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "fgets");
	return;
    }

    fileNum = atoi(transformArgumentToScalar(&args[0])->v.scalar.scalar.data);

    if (fileNum < 0 || fileNum >= fileTableSize || fileTable[fileNum] == 0)
    {
	issueError(ERRMAC_INVALID_MACRO_ARG, "fgets", args[0].value.value->v.scalar.scalar.data);
	return;
    }

    file = fileTable[fileNum];
    do
    {
	result = fgetc(file);

	if (result != EOF)
	{
	    OUT_CHAR(ow, result);
	    if (result == '\n')
		return;
	}
    } while (result != EOF);
}

void
builtInFputs (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    int fileNum;

    if (!(numArgs == 2))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "fputs");
	return;
    }

    fileNum = atoi(transformArgumentToScalar(&args[0])->v.scalar.scalar.data);
    if (fileNum < 0 || fileNum >= fileTableSize || fileTable[fileNum] == 0)
    {
	issueError(ERRMAC_INVALID_MACRO_ARG, "fwrite",
		   args[0].value.value->v.scalar.scalar.data);
	return;
    }

    transformArgumentToScalar(&args[1]);

    fwrite(args[1].value.value->v.scalar.scalar.data,
	   1, args[1].value.value->v.scalar.scalar.length,
	   fileTable[fileNum]);
}

void
builtInFeof (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    dynstring ds;
    FILE *file;
    int fileNum;

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "feof");
	return;
    }

    fileNum = atoi(transformArgumentToScalar(&args[0])->v.scalar.scalar.data);

    if (fileNum < 0 || fileNum >= fileTableSize || fileTable[fileNum] == 0)
    {
	issueError(ERRMAC_INVALID_MACRO_ARG, "feof", ds.data, 0);
	return;
    }

    file = fileTable[fileNum];

    if (feof(file))
    {
	OUT_CHAR(ow, '1');
    }
    else
    {
	OUT_CHAR(ow, '0');
    }
}

void
builtInFstat (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    struct stat buf;

    value *theHash = valueNewHash();

    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "fstat");
	return;
    }

    if (stat(transformArgumentToScalar(&args[0])->v.scalar.scalar.data, &buf) == -1)
    {
	OUT_VALUE(ow, theHash);
    }
    else
    {
	static dynstring uidName,
	    gidName,
	    sizeName,
	    blksizeName,
	    blocksName,
	    atimeName,
	    mtimeName,
	    ctimeName;
	static int haveNames = 0;

	char numberString[64];

	if (!haveNames)
	{
	    haveNames = 1;
	    uidName = dsNewFrom("uid");
	    gidName = dsNewFrom("gid");
	    sizeName = dsNewFrom("size");
	    blksizeName = dsNewFrom("blksize");
	    blocksName = dsNewFrom("blocks");
	    atimeName = dsNewFrom("atime");
	    mtimeName = dsNewFrom("mtime");
	    ctimeName = dsNewFrom("ctime");
	}

	sprintf(numberString, "%u", (unsigned int)buf.st_uid);
	valueHashDefine(theHash, &uidName, valueNewScalarFromCString(numberString));

	sprintf(numberString, "%u", (unsigned int)buf.st_gid);
	valueHashDefine(theHash, &gidName, valueNewScalarFromCString(numberString));

	sprintf(numberString, "%lu", (unsigned long)buf.st_size);
	valueHashDefine(theHash, &sizeName, valueNewScalarFromCString(numberString));

	sprintf(numberString, "%lu", (unsigned long)buf.st_blksize);
	valueHashDefine(theHash, &blksizeName, valueNewScalarFromCString(numberString));

	sprintf(numberString, "%lu", (unsigned long)buf.st_blocks);
	valueHashDefine(theHash, &blocksName, valueNewScalarFromCString(numberString));

	sprintf(numberString, "%lu", (unsigned long)buf.st_atime);
	valueHashDefine(theHash, &atimeName, valueNewScalarFromCString(numberString));

	sprintf(numberString, "%lu", (unsigned long)buf.st_mtime);
	valueHashDefine(theHash, &mtimeName, valueNewScalarFromCString(numberString));

	sprintf(numberString, "%lu", (unsigned long)buf.st_ctime);
	valueHashDefine(theHash, &ctimeName, valueNewScalarFromCString(numberString));

	OUT_VALUE(ow, theHash);
    }
}

void
builtInFgetwd (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    char buffer[PATH_MAX];

    if (!(numArgs == 0))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "fgetwd");
	return;
    }

    getcwd(buffer, PATH_MAX);
    OUT_STRING(ow, buffer, strlen(buffer));
}

void
builtInFchdir (int numArgs, macroArgument *args, environment *env, outputWriter *ow)
{
    if (!(numArgs == 1))
    {
	issueError(ERRMAC_WRONG_NUM_ARGS, "fchdir");
	return;
    }

    chdir(transformArgumentToScalar(&args[0])->v.scalar.scalar.data);
}

void
registerFileOps (void)
{
    int fileNum;
    char fileNumString[64];
    dynstring nameString = dsNewFrom("stdin");

    fileNum = getNewFileNum();
    fileTable[fileNum] = stdin;
    ++numOpenFiles;
    sprintf(fileNumString, "%d", fileNum);
    envAddBinding(0, &nameString, valueNewScalarFromCString(fileNumString));

    registerBuiltIn("fopen", builtInFopen, 1, 0, 0);
    registerBuiltIn("fpipe", builtInFpipe, 1, 0, 0);
    registerBuiltIn("fclose", builtInFclose, 1, 0, 0);
    registerBuiltIn("fgets", builtInFgets, 1, 0, 0);
    registerBuiltIn("fputs", builtInFputs, 1, 0, 0);
    registerBuiltIn("feof", builtInFeof, 1, 0, 0);
    registerBuiltIn("fstat", builtInFstat, 1, 0, 0);
    registerBuiltIn("fgetwd", builtInFgetwd, 1, 0, 0);
    registerBuiltIn("fchdir", builtInFchdir, 1, 0, 0);
}
