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

#include "minimapsettings.h"
#include "minimapconstants.h"
#include "minimaptr.h"

#include <coreplugin/dialogs/ioptionspage.h>
#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>
#include <texteditor/displaysettings.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>
#include <utils/layoutbuilder.h>
#include <utils/qtcassert.h>
#include <utils/store.h>

#include <limits>

using namespace Layouting;

namespace Minimap {
namespace Internal {

namespace {
const char minimapPostFix[] = "Minimap";
} // namespace

MinimapSettings::MinimapSettings(QObject *parent)
{
    setSettingsGroup(minimapPostFix);

    m_enabled.setSettingsKey("Enabled");
    m_enabled.setLabelText(Tr::tr("Enabled"));
    m_enabled.setDefaultValue(true);
    m_enabled.setToolTip(Tr::tr("Check to enable Minimap scrollbar"));
    connect(&m_enabled, &Utils::BaseAspect::changed, this, &MinimapSettings::enabledChanged);

    m_width.setSettingsKey("Width");
    m_width.setLabelText(Tr::tr("Width"));
    m_width.setDefaultValue(Constants::MINIMAP_WIDTH_DEFAULT);
    m_width.setRange(1, std::numeric_limits<int>::max());
    m_width.setToolTip(Tr::tr("The width of the Minimap"));
    connect(&m_width, &Utils::BaseAspect::changed, this, &MinimapSettings::widthChanged);

    m_lineCountThreshold.setSettingsKey("LineCountThresHold");
    m_lineCountThreshold.setLabelText(Tr::tr("Line Count Threshold:"));
    m_lineCountThreshold.setDefaultValue(Constants::MINIMAP_MAX_LINE_COUNT_DEFAULT);
    m_lineCountThreshold.setRange(1, std::numeric_limits<int>::max());
    m_lineCountThreshold.setToolTip(
        Tr::tr("Line count threshold where no Minimap scrollbar is to be used"));
    connect(&m_lineCountThreshold,
            &Utils::BaseAspect::changed,
            this,
            &MinimapSettings::lineCountThresholdChanged);

    m_alpha.setSettingsKey("Alpha");
    m_alpha.setLabelText(Tr::tr("Scrollbar slider alpha value"));
    m_alpha.setDefaultValue(Constants::MINIMAP_ALPHA_DEFAULT);
    m_alpha.setRange(0, 255);
    m_alpha.setToolTip(Tr::tr("The alpha value of the scrollbar slider"));
    connect(&m_alpha, &Utils::BaseAspect::changed, this, &MinimapSettings::alphaChanged);

    m_centerOnClick.setSettingsKey("CenterOnClick");
    m_centerOnClick.setLabelText(Tr::tr("Center on click"));
    m_centerOnClick.setDefaultValue(Constants::MINIMAP_CENTER_ON_CLICK_DEFAULT);
    m_centerOnClick.setToolTip(
        Tr::tr("Center viewport on mouse position when clicking and dragging"));
    connect(&m_centerOnClick,
            &Utils::BaseAspect::changed,
            this,
            &MinimapSettings::centerOnClickChanged);

    m_showLineTooltip.setSettingsKey("ShowLineTooltip");
    m_showLineTooltip.setLabelText(Tr::tr("Show line tooltip"));
    m_showLineTooltip.setDefaultValue(Constants::MINIMAP_SHOW_LINE_TOOLTIP_DEFAULT);
    m_showLineTooltip.setToolTip(Tr::tr("Show line range tooltip when interacting with minimap"));
    connect(&m_showLineTooltip,
            &Utils::BaseAspect::changed,
            this,
            &MinimapSettings::showLineTooltipChanged);

    m_pixelsPerLine.setSettingsKey("PixelsPerLine");
    m_pixelsPerLine.setLabelText(Tr::tr("Pixels per line:"));
    m_pixelsPerLine.setDefaultValue(Constants::MINIMAP_PIXELS_PER_LINE_DEFAULT);
    m_pixelsPerLine.setRange(1, std::numeric_limits<int>::max());
    m_pixelsPerLine.setToolTip(Tr::tr("Pixels per line"));
    connect(&m_pixelsPerLine,
            &Utils::BaseAspect::changed,
            this,
            &MinimapSettings::pixelsPerLineChanged);

    setAutoApply(false);
    readSettings();

    // clang-format off
    setLayouter([this] {
        return Column {
            Form {
                m_enabled, br,
                m_centerOnClick, br,
                m_showLineTooltip, br,
                m_width, br,
                m_lineCountThreshold, br,
                m_alpha, br,
                m_pixelsPerLine,
            },
            st,
        };
    });
    // clang-format on
}

MinimapSettings::~MinimapSettings() {}

MinimapSettings *MinimapSettings::instance()
{
    static MinimapSettings instance;
    return &instance;
}

bool MinimapSettings::enabled()
{
    return instance()->m_enabled();
}
int MinimapSettings::width()
{
    return instance()->m_width();
}
int MinimapSettings::lineCountThreshold()
{
    return instance()->m_lineCountThreshold();
}
int MinimapSettings::alpha()
{
    return instance()->m_alpha();
}
bool MinimapSettings::centerOnClick()
{
    return instance()->m_centerOnClick();
}
bool MinimapSettings::showLineTooltip()
{
    return instance()->m_showLineTooltip();
}
int MinimapSettings::pixelsPerLine()
{
    return instance()->m_pixelsPerLine();
}

class MinimapSettingsPage : public Core::IOptionsPage
{
public:
    MinimapSettingsPage()
    {
        setId(Constants::MINIMAP_SETTINGS);
        setDisplayName(Tr::tr("Minimap"));
        setCategory(TextEditor::Constants::TEXT_EDITOR_SETTINGS_CATEGORY);
        setSettingsProvider([] { return MinimapSettings::instance(); });
    }
};

static const MinimapSettingsPage settingsPage;

} // namespace Internal
} // namespace Minimap
