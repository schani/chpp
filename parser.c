/* -*- c -*- */

/*
 * parser.c
 *
 * chpp
 *
 * Copyright (C) 1998 Mark Probst
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

#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "memory.h"
#include "error.h"

#include "parser.h"

static char*
buildSuspects (const char *delimiters)
{
    static char *src = ")]}",
	*dest = "([{";

    int length,
	i,
	j = 0;
    char *suspects;

    if (delimiters == 0)
	return 0;

    length = strlen(delimiters);
    suspects = (char*)memXAlloc(length + 1);

    for (i = 0; i < length; ++i)
    {
	char *pos = strchr(src, delimiters[i]);

	if (pos != 0)
	    suspects[j++] = dest[pos - src];
    }
    suspects[j] = '\0';

    return suspects;
}

static int
legalNumberOfArguments (dynstring *name, int numArgs,
			environment *env, environment *globalEffects)
{
    value *theValue;

    if (envGetBinding(env, name) != 0 && envGetValueForBinding(env, name) == 0)
	return 1;

    theValue = envGetValueForBinding(globalEnvironment, name);
    if (theValue != 0)
    {
	if (theValue->type == VALUE_BUILT_IN)
	    return 1;           /* for now */
	else if (theValue->type == VALUE_LAMBDA)
	{
	    if (numArgs < theValue->v.lambda.numParams + theValue->v.lambda.minVarArgs)
		return 0;
	    if (theValue->v.lambda.maxVarArgs == -1)
		return 1;
	    if (numArgs > theValue->v.lambda.numParams + theValue->v.lambda.maxVarArgs)
		return 0;
	    return 1;
	}
	else
	    return 0;
    }
    return 1;
}

static bcArgument*
parseArguments (inputReader *ir, int *numArgs, value *builtIn,
		environment *env, environment *globalEffects)
{
    bcArgument *firstArg = 0,
	*lastArg = 0;
    list *globalEffectList;

    *numArgs = 0;

    if (builtIn != 0 && builtIn->v.builtIn.globalEffector != 0)
	globalEffectList = listNewPointerList((list*)memXAlloc(sizeof(list)));
    
    do
    {
	bcArgument *arg = (bcArgument*)memXAlloc(sizeof(bcArgument));
	environment *argumentGlobalEffects,
	    *argumentEnv;

	if (builtIn == 0 || builtIn->v.builtIn.globalEffector == 0)
	    argumentGlobalEffects = globalEffects;
	else
	{
	    argumentGlobalEffects = envNew(globalEffects);
	    listAppend(globalEffectList, argumentGlobalEffects);
	}
	if (builtIn != 0 && builtIn->v.builtIn.environmentor != 0)
	    argumentEnv = builtIn->v.builtIn.environmentor(*numArgs, firstArg, env);
	else
	    argumentEnv = env;

	arg->bc = bcRemoveOuterWS(parParseIntoBCUntil(ir, ",)", 1,
						      argumentEnv, argumentGlobalEffects));
	arg->next = 0;

	if (lastArg == 0)
	    firstArg = lastArg = arg;
	else
	    lastArg = lastArg->next = arg;

	++*numArgs;
    } while (THIS_CHAR(ir) != ')');

    if (*numArgs == 1 && firstArg->bc == 0)
    {
	*numArgs = 0;
	firstArg = 0;
    }
    else if (globalEffects != 0 && builtIn != 0 && builtIn->v.builtIn.globalEffector != 0)
	builtIn->v.builtIn.globalEffector(*numArgs, firstArg, globalEffectList, globalEffects);

    return firstArg;
}

static bcSubscript*
parseSubscripts (inputReader *ir, environment *env, environment *globalEffects)
{
    bcSubscript *firstSubscript = 0,
	*lastSubscript = 0;

    while (THIS_CHAR(ir) == '[' || THIS_CHAR(ir) == '{')
    {
	bcSubscript *subscript =
	    (bcSubscript*)memXAlloc(sizeof(bcSubscript));

	if (THIS_CHAR(ir) == '[')
	{
	    subscript->type = BYTECODE_SUBSCRIPT_LIST;
	    subscript->bc = parParseIntoBCUntil(ir, "]", 1, env, globalEffects);
	}
	else
	{
	    subscript->type = BYTECODE_SUBSCRIPT_HASH;
	    subscript->bc = parParseIntoBCUntil(ir, "}", 1, env, globalEffects);
	}
	subscript->next = 0;

	if (THIS_CHAR(ir) == EOF)
	{
	    issueError(ERRMAC_PREMATURE_END);
	    return 0;
	}

	if (lastSubscript == 0)
	    firstSubscript = lastSubscript = subscript;
	else
	    lastSubscript = lastSubscript->next = subscript;

	if (NEXT_CHAR(ir) == EOF) return firstSubscript;
    }

    return firstSubscript;
}

void
parParseUntil (inputReader *ir, const char *delimiters, bytecodeWriter *bcw, int consume,
	       environment *env, environment *globalEffects)
{
    bytecode *bc;
    char *suspects = buildSuspects(delimiters);

    do
    {
	if (consume)
	    if (NEXT_CHAR(ir) == EOF) return;

	if (delimiters != 0)
	    if (strchr(delimiters, THIS_CHAR(ir)))
		return;

	consume = 1;

	if (THIS_CHAR(ir) == metaChar)
	{
	    if (NEXT_CHAR(ir) == EOF) return;

	    switch (THIS_CHAR(ir))
	    {
		case '<' :
		    {
			bytecode *nameOrValue;
			int byRef,
			    subValue;
			bcSubscript *subscripts = 0;

			if (NEXT_CHAR(ir) == EOF)
			{
			    issueError(ERRMAC_PREMATURE_END);
			    return;
			}

			if (THIS_CHAR(ir) == '&')
			{
			    byRef = BYTECODE_MACRO_BYREF;
			    if (NEXT_CHAR(ir) == EOF)
			    {
				issueError(ERRMAC_PREMATURE_END);
				return;
			    }
			}
			else
			    byRef = 0;

			if (THIS_CHAR(ir) == '(')
			{
			    nameOrValue = parParseIntoBCUntil(ir, ")", 1, env, globalEffects);
			    if (THIS_CHAR(ir) == EOF) return;
			    if (NEXT_CHAR(ir) == EOF)
			    {
				issueError(ERRMAC_PREMATURE_END);
				return;
			    }
			    subValue = BYTECODE_MACRO_SUBVALUE;
			}
			else
			{
			    nameOrValue = parParseIntoBCUntil(ir, "([{=>", 0,
							      env, globalEffects);
			    if (THIS_CHAR(ir) == EOF)
			    {
				issueError(ERRMAC_PREMATURE_END);
				return;
			    }
			    subValue = 0;
			}

			subscripts = parseSubscripts(ir, env, globalEffects);
			if (THIS_CHAR(ir) == EOF) return;

			if (THIS_CHAR(ir) == '=')
			{
			    bc = parParseIntoBCUntil(ir, ">", 1, env, globalEffects);
			    if (THIS_CHAR(ir) != '>')
			    {
				issueError(ERRMAC_PREMATURE_END);
				return;
			    }

			    BCW_OUT_ASSIGNMENT(bcw, byRef, nameOrValue, subscripts, bc);

			    if (globalEffects != 0 && !subValue && bcIsString(nameOrValue))
			    {
				dynstring name = bcExecuteIntoDS(nameOrValue, 0);

				if (envGetBinding(env, &name) == 0
				    && envGetBinding(globalEffects, &name) == 0)
				    envAddBinding(globalEffects, &name, 0);
			    }
			}
			else
			{
			    if (!subValue && bcIsString(nameOrValue))
			    {
				dynstring name = bcExecuteIntoDS(nameOrValue, 0);

				if (envGetBinding(env, &name) == 0
				    && (globalEffects == 0
					|| envGetBinding(globalEffects, &name) == 0))
				    issueWarning(WARNMAC_UNBOUND_VARIABLE, name.data);
			    }

			    if (THIS_CHAR(ir) == '(')
			    {
				int numArgs;
				value *builtIn = 0;
				bcArgument *arguments;

				if (!subValue && bcIsString(nameOrValue))
				{
				    dynstring name = bcExecuteIntoDS(nameOrValue, 0);

				    builtIn = envGetValueForBinding(globalEnvironment, &name);
				    if (builtIn != 0 && builtIn->type != VALUE_BUILT_IN)
					builtIn = 0;
				}

				arguments = parseArguments(ir, &numArgs, builtIn,
							   env, globalEffects);

				if (NEXT_CHAR(ir) == EOF)
				{
				    issueError(ERRMAC_PREMATURE_END);
				    return;
				}

				assert(!byRef); /* should signal error */

				if (!subValue && subscripts == 0 && bcIsString(nameOrValue))
				{
				    dynstring name = bcExecuteIntoDS(nameOrValue, 0);

				    if (!legalNumberOfArguments(&name, numArgs,
								env, globalEffects))
					issueWarning(WARNMAC_WRONG_NUM_ARGS, numArgs, name.data);
				}

				BCW_OUT_MACRO(bcw, subValue, nameOrValue, subscripts,
					      numArgs, arguments);
			    }
			    else if (THIS_CHAR(ir) == '>')
			    {
				BCW_OUT_MACRO(bcw, subValue | byRef, nameOrValue, subscripts,
					      -1, 0);
			    }
			    else
				issueError(ERRMAC_UNEXPECTED_CHAR, THIS_CHAR(ir), '>');
			}
		    }
		    break;

		case '\'' :
		    BCW_OUT_BEGIN_QUOTE(bcw);
		    while (1)
		    {
			if (NEXT_CHAR(ir) == EOF)
			{
			    issueError(ERRMAC_PREMATURE_END);
			    return;
			}

			if (THIS_CHAR(ir) == quoteChar)
			{
			    char c;

			    if (NEXT_CHAR(ir) == EOF)
			    {
				issueError(ERRMAC_PREMATURE_END);
				return;
			    }
			    c = THIS_CHAR(ir);
			    switch (THIS_CHAR(ir))
			    {
				case 'n' :
				    c = '\n';
				    break;
				case 't' :
				    c = '\t';
				    break;
			    }
			    BCW_OUT_CHAR(bcw, c);
			}
			else if (THIS_CHAR(ir) == '\'')
			    break;
			else
			    BCW_OUT_CHAR(bcw, THIS_CHAR(ir));
		    }
		    BCW_OUT_END_QUOTE(bcw);
		    break;

		case '[' :
		    bc = parParseIntoBCUntil(ir, "]", 1, env, globalEffects);
		    if (THIS_CHAR(ir) == EOF)
		    {
			issueError(ERRMAC_PREMATURE_END);
			return;
		    }
		    BCW_OUT_ARITHMETIC(bcw, bc);
		    break;

		case '{' :
		    bc = parParseIntoBCUntil(ir, "}", 1, env, globalEffects);
		    if (THIS_CHAR(ir) == EOF)
		    {
			issueError(ERRMAC_PREMATURE_END);
			return;
		    }
		    BCW_OUT_EVALUATION(bcw, bc);
		    break;

		default :
		    if (THIS_CHAR(ir) == metaChar)
		    {
			BCW_OUT_CHAR(bcw, metaChar);
		    }
		    else
		    {
			int byRef;
			dynstring ds = dsNew();

			if (THIS_CHAR(ir) == '&')
			{
			    byRef = BYTECODE_MACRO_BYREF;
			    if (NEXT_CHAR(ir) == EOF)
			    {
				BCW_OUT_CHAR(bcw, metaChar);
				BCW_OUT_CHAR(bcw, '&');
				return;
			    }
			}
			else
			    byRef = 0;

			do
			{
			    if (!isalnum(THIS_CHAR(ir)) && THIS_CHAR(ir) != '_')
				break;
			    dsAppendChar(&ds, THIS_CHAR(ir));
			    if (NEXT_CHAR(ir) == EOF) break;
			} while (THIS_CHAR(ir) != EOF);
			if (ds.length == 0)
			{
			    BCW_OUT_CHAR(bcw, metaChar);
			    if (byRef)
				BCW_OUT_CHAR(bcw, '&');
			    BCW_OUT_CHAR(bcw, THIS_CHAR(ir));
			}
			else
			{
			    bcSubscript *subscripts;

			    if (envGetBinding(env, &ds) == 0
				&& (globalEffects == 0
				    || envGetBinding(globalEffects, &ds) == 0))
			    {
				BCW_OUT_CHAR(bcw, metaChar);
				if (byRef)
				    BCW_OUT_CHAR(bcw, '&');
				BCW_OUT_STRING(bcw, ds.data, ds.length);

				issueWarning(WARNMAC_UNBOUND_VARIABLE, ds.data);

				consume = 0;
			    }
			    else
			    {
				if (THIS_CHAR(ir) == EOF)
				    subscripts = 0;
				else
				{
				    subscripts = parseSubscripts(ir, env, globalEffects);
				    if (subscripts == 0 && THIS_CHAR(ir) == EOF) return;
				}

				if (THIS_CHAR(ir) == '(')
				{
				    int numArgs;
				    bcArgument *arguments;
				    value *builtIn = envGetValueForBinding(globalEnvironment,
									   &ds);
				    
				    if (builtIn != 0 && builtIn->type != VALUE_BUILT_IN)
					builtIn = 0;

				    arguments = parseArguments(ir, &numArgs, builtIn,
							       env, globalEffects);
				    if (THIS_CHAR(ir) == EOF)
				    {
					issueError(ERRMAC_PREMATURE_END);
					return;
				    }

				    assert(!byRef);

				    if (subscripts == 0 &&
					!legalNumberOfArguments(&ds, numArgs,
								env, globalEffects))
					issueWarning(WARNMAC_WRONG_NUM_ARGS, numArgs, ds.data);

				    BCW_OUT_MACRO(bcw, 0, bcNewString(&ds), subscripts,
						  numArgs, arguments);
				}
				else
				{
				    BCW_OUT_MACRO(bcw, byRef, bcNewString(&ds),
						  subscripts, -1, 0);
				    if (THIS_CHAR(ir) == EOF) return;
				    consume = 0;
				}
			    }
			}
		    }
		    break;
	    }
	}
	else
	{
	    if (suspects != 0 && strchr(suspects, THIS_CHAR(ir)) != 0)
		issueWarning(WARNMAC_SUSPECT_CHARACTER, THIS_CHAR(ir));

	    BCW_OUT_CHAR(bcw, THIS_CHAR(ir));
	}
    } while (THIS_CHAR(ir) != EOF);
}

bytecode*
parParseIntoBCUntil (inputReader *ir, const char *delimiters, int consume,
		     environment *env, environment *globalEffects)
{
    bytecodeWriter *bcw = bcwNewBytecode();

    parParseUntil(ir, delimiters, bcw, consume, env, globalEffects);

    return bcwBytecodeBytecode(bcw);
}
