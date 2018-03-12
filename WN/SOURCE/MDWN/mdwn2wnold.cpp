/***************************************************************************
 *   Copyright (C) 2006 by Marco Beccuti   *
 *   marco@Portatile.network   *
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
#include "./mdwnclass.h"

void open_nets(ifstream& in,ofstream& out,const std::string& net1,std::string net2)
{
//read the components
in.open(net1.c_str(),ifstream::in);
if( !*in) 
	{
    	cerr << "Error opening input stream .net" << endl;
    	throw (ExceptionIO());
  	}

out.open(net2.c_str(),ios::trunc);
if( !*out) 
	{
    	cerr << "Error opening output stream .net" << endl;
    	throw (ExceptionIO());
  	}
}

int main (int argc, char *argv[])
{
map <std::string,class component>CompAtt,CompPas;

if ((argv[1]==NULL)&&(argc<3))
		{
		std::cerr<<"\n\nUSE: MDWN2WN probabilistic_net, non_deterministic_net\n\n";
		exit(1);
		}
//read the components
std::string net= std::string(argv[1])+".def";
ifstream* in=new ifstream(net.c_str(),ifstream::in);
cout<<"Working on the probabilistic net"<<endl;
if( !*in) 
	{
    	cerr << "Error opening input stream .def" << endl;
    	exit(1);
  	}
try
	{
	readComponent(in,CompAtt,CompPas);
	}
catch(ExceptionIO)
	{
	cerr << "Error no active components.\nYou have to insert at least one active component!"<<endl;
	exit(1);
	};

in->close();

//read the components
net= std::string(argv[1])+".net";
in=new ifstream(net.c_str(),ifstream::in);
if( !*in) 
	{
    	cerr << "Error opening input stream .net" << endl;
    	exit(1);
  	}
net= std::string(argv[1])+"M.net";
ofstream* out=new ofstream(net.c_str(),ios::trunc);
if( !*out) 
	{
    	cerr << "Error opening output stream .net" << endl;
    	exit(1);
  	}
//read  prob net --> build prob net + place run and stop
try
	{
	create_net(in,out,CompAtt,CompPas,_PR);
	}
catch(ExceptionIO)	
	{
	cerr << "Error during the creation of the new probabilistic .net"<<endl;
	exit(1);
	};
//read  prob net --> build prob net + place run and stop
out->close();

cout<<"Probabilistic net was generated correctly\nWorking on the non deterministic net"<<endl;

net= std::string(argv[2])+".net";
in=new ifstream(net.c_str(),ifstream::in);
if( !*in) 
	{
    	cerr << "Error opening input stream .net" << endl;
    	exit(1);
  	}
net= std::string(argv[2])+"M.net";
out=new ofstream(net.c_str(),ios::trunc);
if( !*out) 
	{
    	cerr << "Error opening output stream .net" << endl;
    	exit(1);
  	}
//read  prob net --> build prob net + place run and stop
try
	{
	create_net(in,out,CompAtt,CompPas,_ND);
	}
catch(ExceptionIO)	
	{
	cerr << "Error during the creation of the new non deterministic .net"<<endl;
	exit(1);
	};
//read  prob net --> build prob net + place run and stop
out->close();
}
