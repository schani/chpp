/* -*- c -*- */

/*
 * main.c
 *
 * chpp
 *
 * Copyright (C) 1997-1999 Mark Probst
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

#include "config.h"

#include <sys/param.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif

#include "memory.h"
#include "getopt.h"
#include "filler.h"
#include "builtins/builtins.h"
#include "input.h"
#include "output.h"
#include "internals.h"
#include "error.h"
#include "depends.h"
#include "environment.h"
#include "bcoutput.h"
#include "parser.h"

extern dynstring currentFileName;

dynstring mainFileName,
    outputFileName;

char *executableName;

outputWriter toplevelOutputWriter;
inputReader toplevelInputReader;

int generateDependencies = 0;

#ifdef NOT_HEINZI
int
fillBuffer (char *buffer, int max)
{
    return fread(buffer, 1, max, stdin);
}
#endif

int
main (int argc, char *argv[])
{
    FILE *outputFile;
    int haveOutput = 0;
    int testFillBuffer = 0;
    char theCommandChar = '#';
    int precompile = 0;

    executableName = argv[0];

    initEnvironments();

    registerBuiltIns();
    registerInternals();

    defDirs = (dynstring*)memXAlloc(256 * sizeof(dynstring));
    defDirs[0] = dsNewFrom(CHPP_INCLUDEDIR "/");
    nrOfDefDirs = 1;

    while (1)
    {
	static struct option longOptions[] =
	    {
		{ "version", no_argument, 0, 256 },
		{ "help", no_argument, 0, 257 },
		{ "output", required_argument, 0, 'o' },
		{ "test-fillbuffer", no_argument, 0, 258 },
		{ "generate-dependencies", no_argument, 0, 'M' },
		{ "include-dir", required_argument, 0, 'I' },
		{ "command-char", required_argument, 0, 259 },
		{ "meta-char", required_argument, 0, 260 },
		{ "precompile", no_argument, 0, 'P' },
		{ 0, 0, 0, 0 }
	    };

	int option,
	    optionIndex;

	option = getopt_long(argc, argv, "o:D:I:M", longOptions, &optionIndex);

	if (option == -1)
	    break;

	switch (option)
	{
	    case 'o' :
		if (haveOutput)
		{
		    issueError(ERRFRNT_MULTI_OUTPUT, 0, 0, 0);
		    exit(1);
		}
		outputFileName = dsNewFrom(optarg);
		haveOutput = 1;
		break;

	    case 'D' :
		{
		    char *p = strchr(optarg, '=');

		    if (p == 0)
		    {
			dynstring name = dsNewFrom(optarg),
			    value = dsEmpty();

			envAddBinding(0, &name, valueNewScalar(&value));
		    }
		    else
		    {
			dynstring name = dsNewFromBytes(optarg, p - optarg),
			    value = dsNewFrom(p + 1);

			envAddBinding(0, &name, valueNewScalar(&value));
		    }
		}
	        break;

	    case 'I' :
	        {
		    int length = strlen(optarg);

		    assert(length > 0);
		    if (optarg[0] != '/')
		    {
			char currentPath[PATH_MAX];

			getcwd(currentPath, PATH_MAX);
			defDirs[nrOfDefDirs] = dsNewFrom(currentPath);
			dsAppendChar(&defDirs[nrOfDefDirs], '/');
		    }
		    else
			defDirs[nrOfDefDirs] = dsNew();

		    dsAppendString(&defDirs[nrOfDefDirs], optarg, strlen(optarg));
		    dsAppendChar(&defDirs[nrOfDefDirs++], '/');
	        }
		break;

	    case 'M' :
		generateDependencies = 1;
		break;

	    case 'P' :
		precompile = 1;
		break;
		
	    case 256 :
		printf("chpp " VERSION "\n"
		       "\n"
		       "This program is free software; you can redistribute it and/or modify\n"
		       "it under the terms of the GNU General Public License as published by\n"
		       "the Free Software Foundation; either version 2 of the License, or\n"
		       "(at your option) any later version.\n"
		       "\n"
		       "This program is distributed in the hope that it will be useful,\n"
		       "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		       "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		       "GNU General Public License for more details.\n"
		       "\n"
		       "You should have received a copy of the GNU General Public License\n"
		       "along with this program; if not, write to the Free Software\n"
		       "Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n");
		exit(0);

	    case 257 :
		printf("Usage: chpp [option ...] [filename ...]\n"
		       "\n"
		       "  --version                    print out version number\n"
		       "  --help                       print this help text\n"
		       "  -o, --output=FILENAME        write output to specified file\n"
		       "  -I, --include-dir=DIR        add DIR to include search list\n"
		       "  -D NAME=VALUE                define variable\n"
		       "  -M, --generate-dependencies  generate dependency information\n"
		       "  --command-char=CHAR          uses CHAR as the command char\n"
		       "  --meta-char=CHAR             uses CHAR as the meta char\n"
		       "  -P, --precompile             produce precompiled output\n"
		       "\n"
		       "Report bugs to chpp@unix.cslab.tuwien.ac.at\n");
		exit(0);

	    case 258 :
		testFillBuffer = 1;
		break;

	    case 259 :
		assert(strlen(optarg) == 1);
		theCommandChar = optarg[0];
		break;

	    case 260 :
		assert(strlen(optarg) == 1);
		metaChar = optarg[0];
		break;
	}
    }

    if (!haveOutput)
	outputFileName = dsNewFrom("-");

    if (generateDependencies)
	toplevelOutputWriter = owNewDummy();
    else
    {
	if (!haveOutput || strcmp(outputFileName.data, "-") == 0)
	    outputFile = stdout;
	else
	{
	    outputFile = fopen(outputFileName.data, "w");

	    if (outputFile == 0)
	    {
		issueError(ERRFRNT_WRONG_OUTPUT, outputFileName.data, strerror(errno), 0, 0);
		exit(1);
	    }
	}

	toplevelOutputWriter = owNewFile(outputFile);
    }

    if (optind < argc)
    {
	int i;

	for (i = optind; i < argc; ++i)
	{
	    FILE *inputFile;

	    if (strcmp(argv[i], "-") == 0)
		inputFile = stdin; /* hae? */
	    else
	    {
		inputFile = fopen(argv[i], "r");
		if (inputFile == 0)
		    issueError(ERRFRNT_WRONG_INPUT, argv[1], strerror(errno), 0, 0);
		else
		{
		    if (testFillBuffer)
		    {
			int result;
			static char buffer[8192];

			initCommands(inputFile, theCommandChar, argv[i], "./");
			do
			{
			    result = fillBuffer(buffer, 8192);
			    if (result > 0)
				OUT_STRING(&toplevelOutputWriter, buffer, result);
			} while (result != -1);
		    }
		    else
		    {
			bytecodeWriter *bcw = bcwNewOutput(globalEnvironment,
							   &toplevelOutputWriter);
			dynstring path = dsNewFrom(argv[i]),
			    dir,
			    file;

			dsSplitPath(&path, &dir, &file);
			if (dir.length == 0)
			    dir = dsNewFrom("./");
			else
			    dsAppendChar(&dir, '/');

			toplevelInputReader = irNewPreprocessor();
			initCommands(inputFile, theCommandChar, file.data, dir.data);
			currentFileName = dsNewFrom(argv[i]);
			mainFileName = dsNewFrom(argv[i]);
			addDependency(0, &mainFileName);

			parParseUntil(&toplevelInputReader, 0, bcw, 1,
				      globalEnvironment, envNew(0));
		    }
		}
	    }
	}
    }
    else
    {
	if (testFillBuffer)
	{
	    int result;
	    static char buffer[8192];

	    initCommands(stdin, theCommandChar, "-", "./");
	    do
	    {
		result = fillBuffer(buffer, 8192);
		if (result > 0)
		    OUT_STRING(&toplevelOutputWriter, buffer, result);
	    } while (result != -1);
	}
	else
	{
	    bytecodeWriter *bcw = bcwNewOutput(globalEnvironment,
					       &toplevelOutputWriter);

	    toplevelInputReader = irNewPreprocessor();
	    initCommands(stdin, theCommandChar, "-", "./");
	    currentFileName = dsNewFrom("-");
	    mainFileName = dsNewFrom("-");
	    addDependency(0, &mainFileName);

	    parParseUntil(&toplevelInputReader, 0, bcw, 1, globalEnvironment, envNew(0));
	}
    }

    if (generateDependencies)
	outputDependencies();

    if (errorsOccured)
	return 1;

    return 0;
}
