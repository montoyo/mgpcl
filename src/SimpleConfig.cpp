/* Copyright (C) 2017 BARBOTIN Nicolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "mgpcl/SimpleConfig.h"
#include "mgpcl/FileIOStream.h"
#include "mgpcl/LineReader.h"

bool m::SimpleConfig::EmptyLine::writeTo(LineOutputStream *los)
{
	return los->writeLine(content);
}

bool m::SimpleConfig::CategoryLine::writeTo(LineOutputStream *los)
{
	return los->write(prefix) && los->put('[') && los->write(category) && los->put(']') && los->writeLine(suffix);
}

bool m::SimpleConfig::AssignLine::writeTo(LineOutputStream *los)
{
	return los->write(prefix) && los->write(key) && los->write(inBetween) && los->write(value) && los->writeLine(suffix);
}

bool m::SimpleConfig::Property::asBool(bool def) const
{
	if(m_line->value.isEmpty())
		return def;

	if(m_line->value.equalsIgnoreCase("false") || m_line->value.equalsIgnoreCase("off"))
		return false;

	for(int i = 0; i < m_line->value.length(); i++) {
		if(m_line->value[i] != '0')
			return true;
	}

	return false;
}

m::SimpleConfig::Category &m::SimpleConfig::operator[] (const String &cat)
{
	if(m_data.hasKey(cat))
		return m_data[cat];

	Category &ret = m_data[cat];
	CategoryLine *line = new CategoryLine;
	line->category = cat;
	m_lines.add(line);

	ret.m_parent = this;
	ret.m_line = line;
	return ret;
}

m::SimpleConfig::Property &m::SimpleConfig::Category::operator[] (const String &key)
{
	if(m_data.hasKey(key))
		return m_data[key];

	Property &ret = m_data[key];
	int idx = m_parent->m_lines.indexOf(m_line);

	AssignLine *line = new AssignLine;
	line->key = key;
	line->inBetween.append('=', 1);
	m_parent->m_lines.insert(idx + 1, line);

	ret.m_parent = this;
	ret.m_line = line;
	return ret;
}

m::SimpleConfig::Category::Category() : m_data(16)
{
	m_parent = nullptr;
	m_line = nullptr;
}

m::SimpleConfig::SimpleConfig() : m_data(16)
{
	m_errLine = -1;
}

m::SimpleConfig::SimpleConfig(const String &fname) : m_fname(fname), m_data(16)
{
	m_errLine = -1;
}

m::SimpleConfig::~SimpleConfig()
{
	for(Line *line : m_lines)
		delete line;
}

bool m::SimpleConfig::save()
{
	if(m_fname.isEmpty())
		return false;

	SSharedPtr<FileOutputStream> fos(new FileOutputStream);
	if(!fos->open(m_fname, FileOutputStream::kOM_Truncate))
		return false;

	LineOutputStream los(fos.staticCast<OutputStream>());
	for(Line *line : m_lines) {
		if(!line->writeTo(&los)) {
			fos->close();
			return false;
		}
	}

	fos->close();
	return true;
}

m::ConfigLoadError m::SimpleConfig::load()
{
	if(m_fname.isEmpty())
		return kCLE_MissingFileName;

	SSharedPtr<FileInputStream> fis(new FileInputStream);
	if(fis->open(m_fname) != FileInputStream::kOE_Success)
		return kCLE_FileNotFound;

	for(Line *line : m_lines)
		delete line;

	int err;
	int cline = 0;
	String ccat;
	LineReader lr(fis.staticCast<InputStream>());
	m_lines.cleanup();

	while((err = lr.next()) > 0) {
		cline++;

		if(!parseLine(lr.line(), ccat)) {
			m_errLine = cline;
			fis->close();
			return kCLE_FormattingError;
		}
	}

	fis->close();
	return err == 0 ? kCLE_None : kCLE_ReadError;
}

#define M_SCFG_VALID_CHAR(chr) (((chr) >= 'A' && (chr) <= 'Z') || ((chr) >= 'a' && (chr) <= 'z') || ((chr) >= '0' && (chr) <= '9') || (chr) == '_' || (chr) == '-')

bool m::SimpleConfig::parseLine(const String &line, String &ccat)
{
	int pos = 0;
	while(pos < line.length() && (line[pos] == ' ' || line[pos] == '\t'))
		pos++;

	if(pos >= line.length() || line[pos] == '#') {
		//Just an empty line or a comment line
		EmptyLine *el = new EmptyLine;
		el->content = line;
		m_lines.add(el);
		return true;
	}

	if(line[pos] == '[') {
		//Category line
		int end = pos + 1;
		while(end < line.length() && M_SCFG_VALID_CHAR(line[end]))
			end++;

		if(end >= line.length() || line[end] != ']')
			return false;

		//Also check for the suffix
		int i = end + 1;
		while(i < line.length() && (line[i] == ' ' || line[i] == '\t'))
			i++;

		if(i < line.length() && line[i] != '#')
			return false;

		//Insert line
		CategoryLine *cl = new CategoryLine;
		cl->prefix = line.substr(0, pos);
		cl->category = line.substr(pos + 1, end);
		cl->suffix = line.substr(end + 1);
		m_lines.add(cl);

		//Add category & set as current
		Category &cat = m_data[cl->category];
		cat.m_parent = this;
		cat.m_line = cl;

		ccat = cl->category;
		return true;
	} else if(!ccat.isEmpty() && M_SCFG_VALID_CHAR(line[pos])) {
		//Assignement line
		int keyEnd = pos + 1;
		while(keyEnd < line.length() && M_SCFG_VALID_CHAR(line[keyEnd]))
			keyEnd++;

		if(keyEnd >= line.length() || (line[keyEnd] != ' ' && line[keyEnd] != '\t' && line[keyEnd] != '='))
			return false;

		int ibEnd = keyEnd;
		bool hadEqual = false;
		while(ibEnd < line.length()) {
			if(line[ibEnd] == '=') {
				if(hadEqual)
					return false;
				else
					hadEqual = true;
			} else if(line[ibEnd] != ' ' && line[ibEnd] != '\t')
				break;

			ibEnd++;
		}

		if(!hadEqual)
			return false;

		//Add line
		AssignLine *al = new AssignLine;
		al->prefix = line.substr(0, pos);
		al->key = line.substr(pos, keyEnd);
		al->inBetween = line.substr(keyEnd, ibEnd);

		if(ibEnd < line.length()) {
			int valEnd = ibEnd + 1;
			while(valEnd < line.length() && line[valEnd] != '#')
				valEnd++;

			if(valEnd >= line.length()) {
				//Couldn't find any comment, so just trim...
				valEnd = line.length() - 1;

				while(valEnd >= ibEnd && (line[valEnd] == ' ' || line[valEnd] == '\t'))
					valEnd--;

				valEnd++;
			}

			al->value = line.substr(ibEnd, valEnd);
			if(valEnd < line.length())
				al->suffix = line.substr(valEnd);
		}

		m_lines.add(al);

		//Add property
		Category &cat = m_data[ccat];
		Property &prop = cat.m_data[al->key];
		prop.m_parent = &cat;
		prop.m_line = al;
		return true;
	}

	return false;
}
