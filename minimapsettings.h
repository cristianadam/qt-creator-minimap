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

#include <QObject>
#include <utils/aspects.h>

namespace Minimap {
namespace Internal {

class MinimapSettings : public Utils::AspectContainer
{
    Q_OBJECT
public:
    explicit MinimapSettings(QObject *parent = nullptr);
    ~MinimapSettings() override;

    static MinimapSettings *instance();

    static bool enabled();
    static int width();
    static int lineCountThreshold();
    static int alpha();
    static bool centerOnClick();
    static bool showLineTooltip();
    static int pixelsPerLine();

signals:
    void enabledChanged();
    void widthChanged();
    void lineCountThresholdChanged();
    void alphaChanged();
    void centerOnClickChanged();
    void showLineTooltipChanged();
    void pixelsPerLineChanged();

private:
    friend class MinimapSettingsPage;

    Utils::BoolAspect m_enabled{this};
    Utils::IntegerAspect m_width{this};
    Utils::IntegerAspect m_lineCountThreshold{this};
    Utils::IntegerAspect m_alpha{this};
    Utils::BoolAspect m_centerOnClick{this};
    Utils::BoolAspect m_showLineTooltip{this};
    Utils::IntegerAspect m_pixelsPerLine{this};
};

} // namespace Internal
} // namespace Minimap
