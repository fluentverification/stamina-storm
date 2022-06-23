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
					) : keyword(keyword)
						, comment(comment)
						, number(number)
						, type(type)
						, function(function)
						, string(string)
					{
						// Intentionally left empty
					}
					QColor keyword;
					QColor comment;
					QColor number;
					QColor type;
					QColor function;
					QColor string;
				};
				class ColorSchemes {
				public:
					inline static ColorScheme darkMode = ColorScheme(
						QColor("#00aaff")
						, QColor("#bcbcbc")
						, QColor("#ff4747")
						, QColor("#3cbc00")
						, QColor("#94e8ff")
						, QColor("#ff9040")
					);
					inline static ColorScheme lightMode = ColorScheme(
						QColor("#006598")
						, QColor("#515151")
						, QColor("#8b2727")
						, QColor("#267300")
						, QColor("#314d54")
						, QColor("#7a451f")
					);
				};
			}
		}
	}
}
