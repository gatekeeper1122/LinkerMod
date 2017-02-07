#include "csv.h"
#include "../common/fs.h"
#include "../common/io.h"

char* strtok_c(char * _Str, const char * _Delim, char ** _Context)
{
	char* str = (_Str != NULL) ? _Str : *_Context;

	char* ctx = *_Context;
	char* c = strstr(ctx, _Delim);
	if (c != NULL)
	{
		for (const char* d = _Delim; *d; d++)
			*c++ = '\0';
		*_Context = c;
		return ctx;
	}
	
	return NULL;
}

CSVStaticTable::CSVStaticTable(void) : buf(NULL)
{
	this->cells.clear();
}

CSVStaticTable::CSVStaticTable(const char* path, bool trim_empty) : buf(NULL)
{
	this->cells.clear();
	this->ReadFile(path);
	if (trim_empty)
		this->Prune();
}

CSVStaticTable::~CSVStaticTable(void)
{
	delete[] buf;
}

const char* CSVStaticTable::FieldName(int field_index) const
{
	_ASSERT(field_index < this->FieldCount());
	return this->cells[0][field_index];
}

const char* CSVStaticTable::CellValue(int row_index, int field_index) const
{
	_ASSERT(row_index < this->RowCount());
	_ASSERT(field_index < this->FieldCount());

	return this->cells[row_index + 1][field_index];
}

void CSVStaticTable::Prune(void)
{
	int pruned_count = 0;
	for (int r = 0; r < this->RowCount();)
	{
		bool content = false;
		for (int c = 0; c < this->FieldCount(); c++)
		{
			if (strlen(this->cells[r+1][c]))
			{
				content = true;
				r++;
				break;
			}
		}

		if (!content)
		{
			this->cells.erase(this->cells.begin() + r + 1);
			pruned_count++;
		}
	}

	if (pruned_count)
		Con_Printf_v("Pruned %d empty entries from table\n", pruned_count);
}

int CSVStaticTable::ReadFile(const char* path)
{
	int size = FS_FileSize(path);

	this->buf = new char[size + 1];
	memset(buf, 0, size + 1);

	FILE* f;
	fopen_s(&f, path, "rb"); // text mode doesnt like to read the trailing \r\n
	if (!f)
	{
		Con_Error("Unable to open file '%s' for reading\n", path);
		delete[] buf;
		this->buf = NULL;
		return 1;
	}

	int read = fread(buf, 1, size, f);
	fclose(f);

	char* context = buf;
	char* tok = strtok_c(buf, "\r\n", &context);
	for (int i = 0; tok; i++)
	{
		cells.resize(i+1);
		std::vector<const char*>& row = cells[cells.size() - 1];

		char* nt = tok;
		char* tk = strtok_c(tok, ",", &nt);
		for (int t = 0; tk; t++)
		{
			row.push_back(tk);
			tk = strtok_c(NULL, ",", &nt);
		}

		row.push_back(nt);

		if (row.size() != cells[0].size())
		{
			Con_Error("Error: Incorrect number of fields on row %d - found %d, expected %d\n", i, row.size(), cells[0].size());
			this->cells.clear();
			delete[] this->buf;
			this->buf = NULL;
			return 2;
		}

		tok = strtok_c(NULL, "\r\n", &context);
	}

	return 0;
}

int CSVStaticTable::WriteFile(const char* path, bool overwrite) const
{
	if (FS_FileExists(path) && !overwrite)
	{
		Con_Error("File '%s' already exists\n", path);
		return 1;
	}

	FILE* f;
	fopen_s(&f, path, "w"); // text mode doesnt like to read the trailing \r\n
	if (!f)
	{
		Con_Error("Unable to open file '%s' for writing\n", path);
		return 2;
	}

	this->PrintTable(f, false);

	fclose(f);
	return 0;
}

void CSVStaticTable::PrintTable(FILE* h, bool include_debug_info) const
{
	for (unsigned int i = 0; i < cells.size(); i++)
	{
		if (include_debug_info)
			fprintf(h, "[%d]: ", i);

		char delim = ',';
		for (unsigned int c = 0; c < cells[i].size(); c++)
		{
			if (c + 1 >= cells[i].size())
				delim = '\0';

			fprintf(h, "%s%c", cells[i][c], delim);
		}
		fprintf(h, "\n");
	}
}

//void CSV_TEST()
//{
//	const char* path = "D:\\SteamLibrary\\steamapps\\common\\Call of Duty Black Ops\\raw\\soundaliases\\globals\\metadata.csv";
//	const CSVStaticTable table(path);
//	//table.ReadFile(path);
//	table.PrintTable();
//
//	int i = 0;
//}
