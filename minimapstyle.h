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

#pragma once

#include <QProxyStyle>

namespace TextEditor {
class BaseTextEditor;
}

namespace Minimap {
namespace Internal {
class MinimapStyleObject;

class MinimapStyle : public QProxyStyle
{
    Q_OBJECT
public:
    MinimapStyle(QStyle *style);

    void drawComplexControl(ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget = Q_NULLPTR) const override;

    // need this due to QTBUG-24279
    SubControl hitTestComplexControl(ComplexControl control,
                                     const QStyleOptionComplex *option,
                                     const QPoint &pos,
                                     const QWidget *widget = Q_NULLPTR) const override;

    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option = Q_NULLPTR,
                    const QWidget *widget = Q_NULLPTR) const override;

    QRect subControlRect(ComplexControl cc,
                         const QStyleOptionComplex *opt,
                         SubControl sc,
                         const QWidget *widget) const override;

    static QObject *createMinimapStyleObject(TextEditor::BaseTextEditor *editor);

private:
    bool drawMinimap(const QStyleOptionComplex *,
                     QPainter *,
                     const QWidget *,
                     MinimapStyleObject *) const;
};
} // namespace Internal
} // namespace Minimap
