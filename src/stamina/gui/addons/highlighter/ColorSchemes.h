/**
 * STAMINA - the [ST]ochasic [A]pproximate [M]odel-checker for [IN]finite-state [A]nalysis
 * Copyright (C) 2023 Fluent Verification, Utah State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 *
 **/

#include <QColor>

namespace stamina {
	namespace gui {
		namespace addons {
			namespace highlighter {
				class ColorScheme {
				public:
					ColorScheme(
						QColor keyword
						, QColor comment
						, QColor number
						, QColor type
						, QColor function
						, QColor string
						, QColor constant
					) : keyword(keyword)
						, comment(comment)
						, number(number)
						, type(type)
						, function(function)
						, string(string)
						, constant(constant)
					{
						// Intentionally left empty
					}
					QColor keyword;
					QColor comment;
					QColor number;
					QColor type;
					QColor function;
					QColor string;
					QColor constant;
				};
				class ColorSchemes {
				public:
					inline static ColorScheme darkMode = ColorScheme(
						QColor("#00aaff")
						, QColor("#919191")
						, QColor("#ff4747")
						, QColor("#0090bc")
						, QColor("#629aa8")
						, QColor("#ff9040")
						, QColor("#3cbc00") // QColor("#ff58bf")
					);
					inline static ColorScheme lightMode = ColorScheme(
						QColor("#006598")
						, QColor("#515151")
						, QColor("#8b2727")
						, QColor("#005a73")
						, QColor("#314d54")
						, QColor("#7a451f")
						, QColor("#267300") // QColor("#461835")
					);
				};
			}
		}
	}
}
