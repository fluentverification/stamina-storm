#ifndef STAMINA_GUI_ADDONS_HIGHLIGHTER_PRISMHIGHLIGHTER_H
#define STAMINA_GUI_ADDONS_HIGHLIGHTER_PRISMHIGHLIGHTER_H

#include "Highlighter.h"

namespace stamina {
	namespace gui {
		namespace addons {
			namespace highlighter {
				class PrismHighlighter : public Highlighter {
					Q_OBJECT
				public:
					PrismHighlighter(QTextDocument * parent = nullptr);
				protected:
					// Rules
					void setupKeyWordPatterns() override;

				};
			}
		}
	}
}

#endif // STAMINA_GUI_ADDONS_HIGHLIGHTER_H
