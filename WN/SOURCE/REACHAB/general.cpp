/***************************************************************************
 *   Copyright (C) 2007 by Marco Beccuti   *
 *   beccuti@mfn.unipmn.it   *
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


#include "general.h" 


namespace general{

Parser::Parser(const std::string& delims, char* buffer)
{
this->delims=delims;
char* stoken=strtok(buffer,this->delims.c_str());
while (stoken!=NULL)
	{
	token.push_back(stoken);
	stoken=strtok(NULL,this->delims.c_str());
	}
}

Parser::Parser(const std::string& delims, const std::string& buffer)
{
this->delims=delims;
//per non modificare la stringa
std::string buffer1(buffer,0,buffer.size());
//per non modificare la stringa
char* stoken=strtok((char*)buffer1.c_str(),this->delims.c_str());
while (stoken!=NULL)
	{
	token.push_back(stoken);
	stoken=strtok(NULL,this->delims.c_str());
	}
}


void Parser::update(const std::string& delims, const std::string& buffer)
{
this->delims=delims;
token.clear();
std::string buffer1(buffer,0,buffer.size());
char* stoken=strtok((char*)buffer1.c_str(),this->delims.c_str());
while (stoken!=NULL)
	{
	token.push_back(stoken);
	stoken=strtok(NULL,this->delims.c_str());
	}
}

void Parser::update(const std::string& delims, char* buffer)
{
this->delims=delims;
token.clear();

char* stoken=strtok((char*)buffer,this->delims.c_str());
while (stoken!=NULL)
	{
	token.push_back(stoken);
	stoken=strtok(NULL,this->delims.c_str());
	}
}

void Parser::set(const int& i,std::string newstr)
{
token[i]=newstr;
}

int Parser::find(const std::string& test)
{
for (int i=0;i<(int)token.size();i++)
	{
	if (token[i]==test)
		{
		return i;
		}
	}
return -1;
}

int Parser::findD(const std::string& test,const std::string& test2)
{
for (int i=0;i<(int)token.size();i++)
	{
	if ((token[i]==test)||(token[i]==test2))
		{
		return i;
		}
	}
return -1;
}

int Parser::findS(const std::string& test)
{
int size= (int) test.size();
string comp="";
for (int i=0;i<(int)token.size();i++)
	{
	comp=token[i].substr(0,size);
	if (comp==test)
		{
		return i;
		}
	}
return -1;
}

std::string Parser::get(int i) 
{
if ((i>=0)&&(i<(int)token.size())) 
	return token[i]; 
else 
	return "";
}

ostream& operator<<(ostream& out,class Parser& data)	
		{
		out<<"-----------------------------------------";
		out<<"\nField  delims: "<<data.delims<<endl;
		for (int i=0;i<(int)data.token.size();i++)
			{
			out <<"Token["<<i<<"]: \""<<data.token[i]<<"\""<<endl;
			}
		out<<"\n-----------------------------------------\n";
		return out;
		}


std::string Parser::get_string()
{
std::string comp="";
for (int i=0;i<(int)token.size();i++)
	{
	comp+=token[i];
	}
return comp;
}

std::string Parser::get_string(char delim)
{
std::string comp="";
if (token.size()>0)  comp=token[0];

for (int i=1;i<(int)token.size();i++)
	{
	comp+=delim+token[i];
	}
return comp;
}
ParserB::ParserB(const std::string& delims, const std::string& buffer)
{

Parser();

this->delims=delims;
string temp="";
unsigned int i=0;
while (i<buffer.size())
	{
	if (buffer[i]!=delims[0])
		{
		temp+=buffer[i];
		}
	else
		{
		unsigned int k=1;
		bool fine=false;
		while ((k<delims.size())&&(!fine))
			{
			 if (buffer[i+k]!=delims[k])
				{
				fine=true;
				}
			k++;
			}
		if (fine==false)
			{
			token.push_back(temp);
			temp="";
			i=i+delims.size()-1;
			}
		else
			{
			temp+=buffer[i];
			}
		}
	i++;
	}
token.push_back(temp);
}

void ParserB::update(const std::string& delims, const std::string& buffer)
{
this->delims=delims;
string temp="";
unsigned int i=0;
while (i<buffer.size())
	{
	if (buffer[i]!=delims[0])
		{
		temp+=buffer[i];
		}
	else
		{
		unsigned int k=1;
		bool fine=false;
		while ((k<delims.size())&&(!fine))
			{
			 if (buffer[i+k]!=delims[k])
				{
				fine=true;
				}
			k++;
			}
		if (fine==false)
			{
			token.push_back(temp);
			temp="";
			i=i+delims.size()-1;
			}
		else
			{
			temp+=buffer[i];
			}
		}
	i++;
	}
token.push_back(temp);
}

}




