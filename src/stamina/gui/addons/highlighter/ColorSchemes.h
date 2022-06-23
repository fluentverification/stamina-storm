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
					) : keyword(keyword)
						, comment(comment)
						, number(number)
						, type(type)
						, function(function)
					{
						// Intentionally left empty
					}
					QColor keyword;
					QColor comment;
					QColor number;
					QColor type;
					QColor function;
				};
				class ColorSchemes {
					static ColorScheme darkMode(
						QColor("#00aaff")
						, QColor("#bcbcbc")
						, QColor("#ff4747")
						, QColor("#3cbc00")
						, QColor("#94e8ff")
					);
					static ColorScheme lightMode(
						QColor("#006598")
						, QColor("#515151")
						, QColor("#8b2727")
						, QColor("#267300")
						, QColor("#314d54")
					);
				};
			}
		}
	}
}
