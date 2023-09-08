#ifndef STAMINA_GUI_ADDONS_LEXER
#define STAMINA_GUI_ADDONS_LEXER

#include "CodeEditor.h"

namespace stamina {
	namespace gui {
		namespace addons {
			class LexerNode : public Widget {
				Q_OBJECT
			public:
				/**
				 * Basic constructor. Creates a LexerNode with a given parent
				 * node and an associated
				 * */
				LexerNode(
					LexerNode * parent
					, QTextBlock * block
					, QList<QTextBlock *> * collapsibleText = nullptr
				);
				QTextBlock * getBlock();
				void setBlock(QTextBlock * block);
				void addSubNode(LexerNode * node);
				QList<LexerNode *> subNodes;
			private:
				uint16_t lineNumber; // The line number associated with
									 // the start of this code block
				QTextBlock * block;  // The text block associated with this node
				QList<QTextBlock *> * collapsibleText; // The text which can be collapsed
													   // by this node
			};
			class Lexer : public QObject {
				Q_OBJECT
			public:
				Lexer(LineNumberArea * parent, CodeEditor * editor);
			public slots:
				/**
				 * The be-all end-all function of this class. It
				 * is a pure-virtual function which must be overrode
				 * by sub-classes. Some notes:
				 *     - You only have to lex from where the edit occurs
				 *     - If the blocks haven't changed, you can probably return before finishing lex
				 * */
				void lex() = 0;
				void handleEdits(/* TODO: figure out what kind of QEvent */ * event);
			private:
				LineNumberArea * parent;
				CodeEditor * editor;
			};
		}
	} // namespace gui
} // namespace stamina
#endif // STAMINA_GUI_ADDONS_LEXER
