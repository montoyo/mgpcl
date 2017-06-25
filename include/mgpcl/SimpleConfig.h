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

#pragma once
#include "LineOStream.h"
#include "HashMap.h"

namespace m
{
	enum ConfigLoadError
	{
		kCLE_None = 0,
		kCLE_MissingFileName,
		kCLE_FileNotFound,
		kCLE_ReadError,
		kCLE_FormattingError
	};

	//Simple config can read very simple INI-like configuration files.
	//The benefit from using this instead of JSON (for instance) is that
	//it does not override the user's formatting.
	class SimpleConfig
	{
		M_NON_COPYABLE(SimpleConfig)

	private:
		class Line
		{
		public:
			virtual ~Line()
			{
			}

			virtual bool writeTo(LineOutputStream *los) = 0;
		};

		class EmptyLine : public Line
		{
		public:
			//Line with just some blanks and a comment
			String content;
			bool writeTo(LineOutputStream *los) override;
		};

		class CategoryLine : public Line
		{
		public:
			String prefix; //Blanks
			String category; //[category]
			String suffix; //Blanks and/or comment

			bool writeTo(LineOutputStream *los) override;
		};

		class AssignLine : public Line
		{
		public:
			String prefix; //Blanks
			String key; //key
			String inBetween; //blanks=blanks
			String value; //value
			String suffix; //Blanks and/or comment

			bool writeTo(LineOutputStream *los) override;
		};

	public:
		class Category;

		//Do not keep a Property outside the scope of its corresponding SimpleConfig.
		class Property
		{
			friend class Category;
			friend class SimpleConfig;
			friend class HashMap<String, Property>;

		public:
			const String &key() const
			{
				return m_line->key;
			}

			const String &value() const
			{
				return m_line->value;
			}

			const String &value(const String &strIfEmpty) const
			{
				return m_line->value.isEmpty() ? strIfEmpty : m_line->value;
			}

			int asInt(int def = 0) const
			{
				return m_line->value.isEmpty() ? def : m_line->value.toInteger();
			}

			double asDouble(double def = 0.0) const
			{
				return m_line->value.isEmpty() ? def : m_line->value.toDouble();
			}

			bool asBool(bool def = false) const;

			void setValue(const String &val)
			{
				m_line->value = val;
			}

			void setIntValue(int val)
			{
				m_line->value = String::fromInteger(val);
			}

			void setDoubleValue(double val)
			{
				m_line->value = String::fromDouble(val, 8);
			}

			void setBool10(bool val)
			{
				m_line->value = val ? "1" : "0";
			}

			void setBoolTF(bool val)
			{
				m_line->value = val ? "true" : "false";
			}

			void setBoolOO(bool val)
			{
				m_line->value = val ? "on" : "off";
			}

			bool isEmpty() const
			{
				return m_line->value.isEmpty();
			}

			Category &category()
			{
				return *m_parent;
			}

		private:
			Property()
			{
				m_parent = nullptr;
				m_line = nullptr;
			}

			Category *m_parent;
			AssignLine *m_line;
		};

		//Do not keep a Category outside the scope of its corresponding SimpleConfig.
		class Category
		{
			friend class SimpleConfig;
			friend class HashMap<String, Category>;

		public:
			const String &name() const
			{
				return m_line->category;
			}

			Property &operator[](const String &key);

			SimpleConfig &config()
			{
				return *m_parent;
			}

		private:
			Category();

			SimpleConfig *m_parent;
			CategoryLine *m_line;
			HashMap<String, Property> m_data;
		};

		SimpleConfig();
		SimpleConfig(const String &fname);
		~SimpleConfig();

		bool save();
		ConfigLoadError load();

		int erroringLine() const
		{
			return m_errLine;
		}

		const String &fileName() const
		{
			return m_fname;
		}

		void setFileName(const String &fname)
		{
			m_fname = fname;
		}

		Category &operator[](const String &cat);

	private:
		bool parseLine(const String &line, String &ccat);

		String m_fname;
		int m_errLine;
		List<Line*> m_lines;
		HashMap<String, Category> m_data;
	};
}
