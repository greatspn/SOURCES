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
#include <set>
#include <map>
#include <stack>
#include <list>
#include <fstream>
#include <sstream>

using namespace  general;

class el {
public:
    string id;
    string t;
    el(string id, string t) { this->id = id; this->t = t;};
    ~el() {};
};


int main(int argc, char *argv[]) {
    Parser parser, parser1, parser2;
    char delim[] = " ", delim1[] = "()";

    if (argv[1] == NULL || argc < 5 || argv[2] == NULL) {
        std::cerr << "\n\nUSE: PARSER  <netpath>/net.srg <state> <deep path> <string>\n\n";
        exit(EXIT_FAILURE);
    }


//std::string tmp = std::string(argv[1])+".dot";
    std::ifstream in(argv[1], std::ifstream::in);
    if (!in) {
        cout << "Error: I cannot open the file " << argv[1] << endl;
        exit(EXIT_FAILURE);
    }

    stack <string> st;
    st.push(argv[2]);
    string buffer = "";
    string tmp = "", tmp1 = "", tmp2 = "", tmp3 = "";
    map <string, string> mark_desc;
    map <string, list <class el> > h_reac;
    getline(in, buffer, '\n');
    getline(in, buffer, '\n');
    mark_desc["T1"] = buffer;
    parser2.update(delim1, buffer);
    cout << "Marking list:\n";
    if (parser2.findS(string(argv[4])) != -1)
        cout << "T1:" << buffer << endl;
    while (!in.eof()) {
        getline(in, buffer, '\n');
        parser.update(delim, buffer);
        if (parser.get(0) == "MARKING" && parser.size() == 3) {
            getline(in, buffer, '\n');
            getline(in, tmp1, '\n'); getline(in, tmp1, '\n');
            parser1.update(delim, tmp1);
            getline(in, tmp2, '\n');
            h_reac[parser1.get(1)].push_back(el(parser.get(1), buffer));
            //h_reac[parser1.get(1)].id=buffer;
            if (mark_desc.find(parser1.get(1)) == mark_desc.end()) {
                mark_desc[parser1.get(1)] = tmp2;
                parser2.update(delim1, tmp2);
                if (parser2.findS(string(argv[4])) != -1) {
                    //cout<<parser1.get(1)<<":"<<tmp2<<endl;
                    bool ok = true;
                    for (unsigned int i = 1; i <= parser2.size(); i = i + 2) {
                        if (parser2.get(i) != string(argv[4])) {
                            ok = false;
                        }
                    }
                    if (ok) {
                        cout << "****" << parser1.get(1) << ":" << tmp2 << endl;
                    }
                    else
                        cout << parser1.get(1) << ":" << tmp2 << endl;
                }
            }
        }
    }
    cout << "---------------------------------\n\n";
    int level = atoi(argv[3]);
    while (!st.empty() && (level != 0)) {
        tmp = st.top();
        st.pop();
        cout << "\n" << tmp << ":\t\t" << mark_desc[tmp] << "\n";
        list <class el>::iterator iter;
        iter = h_reac[tmp].begin();
        while	(iter != h_reac[tmp].end()) {
            cout << "  <--" << iter->id << ":\t" << mark_desc[iter->id] << "\t" << iter->t << ")] " << endl;
            st.push(iter->id);
            iter++;
        }
        level--;
    }
}


/*			h_rech[parser1.get(1)].id=parser.get(1);
			if (mark_desk.find(parser.get(1))==mark_desk.end())
					{
				        tmp1=parser.update(delim,buffer);
					mark_desk[parser.get(1)]=tmp1;
					}

			}
		}




map <string,string> mark_tran;
while (!(ls.empty()))
	{
	tmp=ls.front();
	ls.pop_front();
	//ifstream pos=in.tellg();
	while(!in.eof())
		{
		getline(in,buffer,'\n');
		parser.update(delim,buffer);
		if (parser.get(1)==tmp && parser.size()<3)
			{
			getline(in,tmp3,'\n');
			if ((tmp1!="")&&(tmp2!=""))
				{
				ls.push_front(tmp1);
				mark_tran[tmp1]=tmp2+")]";
				tmp1="";
				tmp2="";
				}
			}
		else
			{
			if  (parser.get(0)=="MARKING")
				{
				tmp1=parser.get(1);
				getline(in,tmp2,'\n');
				}
			}
		}
	map <string, string>::iterator iter=mark_tran.begin();
	cout<<tmp<<":\t"<<tmp3<<"\n";
	tmp3="";
	while (iter!=mark_tran.end())
		{
		cout<<"<--\t"<<iter->second<<"\n\t"<<iter->first<<endl;
		iter++;
		}
	mark_tran.clear();
	cout<<endl<<endl;
	in.seekg(0, ios::beg);
	}

}
*/


