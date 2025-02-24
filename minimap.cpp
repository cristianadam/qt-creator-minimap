/*
  Minimap QtCreator plugin.

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not see
  http://www.gnu.org/licenses/lgpl-2.1.html.

  Copyright (c) 2017, emJay Software Consulting AB, See AUTHORS for details.
*/

#include "minimap.h"
#include "minimapsettings.h"
#include "minimapstyle.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <texteditor/texteditor.h>

#include <QApplication>

namespace Minimap {
namespace Internal {

MinimapPlugin::MinimapPlugin() {}

MinimapPlugin::~MinimapPlugin()
{
    MinimapStyle *style = qobject_cast<MinimapStyle *>(qApp->style());
    if (style) {
        qApp->setStyle(style->baseStyle());
    }
}

bool MinimapPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    new MinimapSettings(this);

    qApp->setStyle(new MinimapStyle(qApp->style()));

    Core::EditorManager *em = Core::EditorManager::instance();
    connect(em, &Core::EditorManager::editorCreated, this, &MinimapPlugin::editorCreated);

    return true;
}

void MinimapPlugin::editorCreated(Core::IEditor *editor, const Utils::FilePath &fileName)
{
    Q_UNUSED(fileName);
    TextEditor::BaseTextEditor *baseEditor = qobject_cast<TextEditor::BaseTextEditor *>(editor);
    if (baseEditor) {
        MinimapStyle::createMinimapStyleObject(baseEditor);
    }
}
} // namespace Internal
} // namespace Minimap
