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

#include "LineNumberArea.h"

namespace stamina {
namespace gui {
namespace addons {

LineNumberArea::LineNumberArea(CodeEditor * editor)
	: QWidget(editor)
	, editor(editor)
{
	// Intentionally left empty
}

void
LineNumberArea::paintEvent(QPaintEvent * event) {
	editor->lineNumberAreaPaintEvent(event);
}

} // namespace addons
} // namespace gui
} // namespace stamina
