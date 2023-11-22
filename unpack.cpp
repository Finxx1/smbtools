/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of SMB Sound and Data unpacker/packer
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Alin Baciu (Edited by Finxx)
    Website:	www.awkwardgames.wordpress.com
*/
#include <iostream>
#include <stdio.h>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
void CreateDirectoryA(const char* dir, void* crap) {
    mkdir(dir, 0777);
}
#endif

using namespace std;

struct dirNfo
{
	int a,
		b;
};
struct dir
{
	dirNfo dnf;
	string name;
};
struct fileNfo
{
	int offset,
		size,
		dir;
};
struct file
{
	fileNfo fnf;
	string name;
};

vector <dirNfo> dnfo;
vector <fileNfo> fnfo;

vector <dir> dirs;
vector <file> files;
void getFileDetails(FILE *f)
{
	//read directory info
	int ndirs;
	fread(&ndirs,sizeof(int),1,f);
	cout<<ndirs<<endl;
	
	for(int i=0;i<ndirs;i++)
	{
		dirNfo buff;
		fread(&buff,sizeof(dirNfo),1,f);
		dnfo.push_back(buff);
	}

	//read files info
	int nfiles;
	fread(&nfiles,sizeof(int),1,f);
	cout<<nfiles<<endl;
	
	for(int i=0;i<nfiles;i++)
	{
		fileNfo buff;
		fread(&buff,sizeof(fileNfo),1,f);
		fnfo.push_back(buff);
	}
	int szdirnames,szfilenames;
	fread(&szdirnames,sizeof(int),1,f);
	fread(&szfilenames,sizeof(int),1,f);

	//read directory names
	char *dirnames = new char[szdirnames];
	fread(dirnames,szdirnames,1,f);

	int dP=0;
	dir bfx;
	bfx.dnf=dnfo[dP];
	dirs.push_back(bfx);
	for(int i=0;i<szdirnames;i++)
	{
		if(dirnames[i]==0 && (dP+1)<ndirs)
		{
			dP++;
			dir bff;
			bff.dnf=dnfo[dP];
			dirs.push_back(bff);
		}
		else
			dirs[dP].name.push_back(dirnames[i]);
	}


	char *filenames = new char[szfilenames];
	fread(filenames,szfilenames,1,f);

	int fP=0;
	file bfy;
	bfy.fnf=fnfo[fP];
	files.push_back(bfy);
	for(int i=0;i<szfilenames;i++)
	{
		if(filenames[i]==0 && (fP+1)<nfiles)
		{
			fP++;
			file bff;
			bff.fnf=fnfo[fP];
			files.push_back(bff);
		}
		else
			files[fP].name.push_back(filenames[i]);
	}
	
}
void extractFiles(FILE *f)
{
	CreateDirectoryA("data",NULL);
	//for(int j=0;j<2;j++)
	{
		for(int i=0;i<dirs.size();i++)
		{
			string pt = "data/";
			pt.append(dirs[i].name);

			CreateDirectoryA(pt.c_str(),NULL);
		}
	}
	for(int i=0;i<files.size();i++)
	{
		cout<<i<<' '<<files[i].name.c_str()<<endl;
		string path = "data/";
		path.append(files[i].name);
		FILE *p = fopen(path.c_str(),"wb");

		fseek(f,files[i].fnf.offset,SEEK_SET);

		for(int j=0;j<files[i].fnf.size;j++)
		{
			char c;
			fread(&c,sizeof(char),1,f);
			fwrite(&c,sizeof(char),1,p);
		}

		fclose(p);
	}
}
void unpackData(string file)
{
	FILE *f = fopen(file.c_str(),"rb");

	getFileDetails(f);
	extractFiles(f);

	fclose(f);
}

int main(int argc, const char* argv[])
{
	printf("Super Meat Boy unpacker\n");
	if(argc==1)
	{
        printf("You gotta specify the file to unpack, nerd.");
	}
	else
	{
        unpackData(argv[1]);
	}
	return 0;
}
