/***************************************************************************
 *   Copyright (C) 2011 by Marco Beccuti   *
 *   beccuti@di.unito.it   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

%option prefix="kk"
%{
#include "AutoParser.yy.h"

extern YYSTYPE yylval;

%} 

%%
"//".*     		/* ignore comment */;
[ \t\n]+                  /* ignore whitespace */;
<<EOF>>			{return -1;}
[0-9]+      		{kklval.num= atoi(yytext); return NUMBER;}
[A-Za-z_][A-Za-z0-9_]*	{strcpy(kklval.var,yytext); return STRING;}
.               	{return (int) yytext[0];}
%%
