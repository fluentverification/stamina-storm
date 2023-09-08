#include "Lexer.h"

namespace stamina {
namespace gui {
namespace addons {

LexerNode::LexerNode(
	LexerNode * parent
	, QTextBlock * block
	, QList<QTextBlock *> * collapsibleText
)
	: parent(parent)
	, block(block)
	, collapsibleText(collapsibleText)
{
	// Intentionally left empty
}

QTextBlock *
LexerNode::getBlock() {
	return block;
}

void
LexerNode::setBlock(QTextBlock * block) {
	this->block = block;
}

void
LexerNode::addSubNode(LexerNode * node) {
	subNodes.add(node);
}

void
LexerNode::collapseText() {
	if (collapsibleText == nullptr) {
		return;
	}

	// TODO
}

void
LexerNode::uncollapseText() {
	if (collapsibleText == nullptr) {
		return;
	}

	// TODO
}

} // namespace addons
} // namespace gui
} // namespace stamina
