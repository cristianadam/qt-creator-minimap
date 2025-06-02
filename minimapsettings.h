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

#include <utils/store.h>

namespace Minimap {
namespace Internal {
class MinimapSettingsPage;

class MinimapSettings : public QObject
{
    Q_OBJECT
public:
    explicit MinimapSettings(QObject *parent);
    ~MinimapSettings();

    Utils::Store toMap() const;
    void fromMap(const Utils::Store &map);

    static MinimapSettings *instance();

    static bool enabled();
    static int width();
    static int lineCountThreshold();
    static int alpha();
    static bool centerOnClick();
    static bool showLineTooltip();

signals:
    void enabledChanged(bool);
    void widthChanged(int);
    void lineCountThresholdChanged(int);
    void alphaChanged(int);
    void centerOnClickChanged(bool);
    void showLineTooltipChanged(bool);

private:
    friend class MinimapSettingsPageWidget;

    void setEnabled(bool enabled);
    void setWidth(int width);
    void setLineCountThreshold(int lineCountThreshold);
    void setAlpha(int alpha);
    void setCenterOnClick(bool centerOnClick);
    void setShowLineTooltip(bool showLineTooltip);

    bool m_enabled;
    int m_width;
    int m_lineCountThreshold;
    int m_alpha;
    bool m_centerOnClick;
    bool m_showLineTooltip;
    MinimapSettingsPage *m_settingsPage;
};
} // namespace Internal
} // namespace Minimap
