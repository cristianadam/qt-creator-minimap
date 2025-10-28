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

#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>
#include <texteditor/displaysettings.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>
#include <utils/qtcassert.h>
#include <utils/store.h>

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPointer>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

#include <limits>

namespace Minimap {
namespace Internal {
namespace {
const char minimapPostFix[] = "Minimap";
const char enabledKey[] = "Enabled";
const char widthKey[] = "Width";
const char lineCountThresholdKey[] = "LineCountThresHold";
const char alphaKey[] = "Alpha";
const char centerOnClickKey[] = "CenterOnClick";
const char showLineTooltipKey[] = "ShowLineTooltip";
const char pixelsPerLineKey[] = "PixelsPerLine";

MinimapSettings *m_instance = 0;
} // namespace

class MinimapSettingsPageWidget : public Core::IOptionsPageWidget
{
public:
    MinimapSettingsPageWidget()
    {
        connect(TextEditor::TextEditorSettings::instance(),
                &TextEditor::TextEditorSettings::displaySettingsChanged,
                this,
                &MinimapSettingsPageWidget::displaySettingsChanged);
        m_textWrapping = TextEditor::TextEditorSettings::displaySettings().m_textWrapping;

        QVBoxLayout *layout = new QVBoxLayout;
        QGroupBox *groupBox = new QGroupBox(this);
        groupBox->setTitle(Tr::tr("Minimap"));
        layout->addWidget(groupBox);
        QFormLayout *form = new QFormLayout;
        m_enabled = new QCheckBox(groupBox);
        m_enabled->setToolTip(Tr::tr("Check to enable Minimap scrollbar"));
        m_enabled->setChecked(m_instance->m_enabled);
        form->addRow(Tr::tr("Enabled:"), m_enabled);
        m_width = new QSpinBox;
        m_width->setMinimum(1);
        m_width->setMaximum(std::numeric_limits<int>::max());
        m_width->setToolTip(Tr::tr("The width of the Minimap"));
        m_width->setValue(m_instance->m_width);
        form->addRow(Tr::tr("Width:"), m_width);
        m_lineCountThresHold = new QSpinBox;
        m_lineCountThresHold->setMinimum(1);
        m_lineCountThresHold->setMaximum(std::numeric_limits<int>::max());
        m_lineCountThresHold->setToolTip(
            Tr::tr("Line count threshold where no Minimap scrollbar is to be used"));
        m_lineCountThresHold->setValue(m_instance->m_lineCountThreshold);
        form->addRow(Tr::tr("Line Count Threshold:"), m_lineCountThresHold);
        m_alpha = new QSpinBox;
        m_alpha->setMinimum(0);
        m_alpha->setMaximum(255);
        m_alpha->setToolTip(Tr::tr("The alpha value of the scrollbar slider"));
        m_alpha->setValue(m_instance->m_alpha);
        form->addRow(Tr::tr("Scrollbar slider alpha value:"), m_alpha);
        m_centerOnClick = new QCheckBox(groupBox);
        m_centerOnClick->setToolTip(
            Tr::tr("Center viewport on mouse position when clicking and dragging"));
        m_centerOnClick->setChecked(m_instance->m_centerOnClick);
        form->addRow(Tr::tr("Center on click:"), m_centerOnClick);
        m_showLineTooltip = new QCheckBox(groupBox);
        m_showLineTooltip->setToolTip(
            Tr::tr("Show line range tooltip when interacting with minimap"));
        m_showLineTooltip->setChecked(m_instance->m_showLineTooltip);
        form->addRow(Tr::tr("Show line tooltip:"), m_showLineTooltip);
        m_pixelsPerLine = new QSpinBox;
        m_pixelsPerLine->setMinimum(1);
        m_pixelsPerLine->setMaximum(std::numeric_limits<int>::max());
        m_pixelsPerLine->setToolTip(Tr::tr("Pixels per line"));
        m_pixelsPerLine->setValue(m_instance->m_pixelsPerLine);
        form->addRow(Tr::tr("Pixels per line:"), m_pixelsPerLine);

        groupBox->setLayout(form);
        setLayout(layout);
        setEnabled(!m_textWrapping);
        setToolTip(m_textWrapping ? Tr::tr("Disable text wrapping to enable Minimap scrollbar")
                                  : QString());
    }

    void apply()
    {
        bool save(false);
        if (m_enabled->isChecked() != MinimapSettings::enabled()) {
            m_instance->setEnabled(m_enabled->isChecked());
            save = true;
        }
        if (m_width->value() != MinimapSettings::width()) {
            m_instance->setWidth(m_width->value());
            save = true;
        }
        if (m_lineCountThresHold->value() != MinimapSettings::lineCountThreshold()) {
            m_instance->setLineCountThreshold(m_lineCountThresHold->value());
            save = true;
        }
        if (m_alpha->value() != MinimapSettings::alpha()) {
            m_instance->setAlpha(m_alpha->value());
            save = true;
        }
        if (m_centerOnClick->isChecked() != MinimapSettings::centerOnClick()) {
            m_instance->setCenterOnClick(m_centerOnClick->isChecked());
            save = true;
        }
        if (m_showLineTooltip->isChecked() != MinimapSettings::showLineTooltip()) {
            m_instance->setShowLineTooltip(m_showLineTooltip->isChecked());
            save = true;
        }
        if (m_pixelsPerLine->value() != MinimapSettings::pixelsPerLine()) {
            m_instance->setPixelsPerLine(m_pixelsPerLine->value());
            save = true;
        }
        if (save) {
            Utils::storeToSettings(Utils::keyFromString(minimapPostFix),
                                   Core::ICore::settings(),
                                   m_instance->toMap());
        }
    }

private:
    void displaySettingsChanged(const TextEditor::DisplaySettings &settings)
    {
        m_textWrapping = settings.m_textWrapping;
        setEnabled(!m_textWrapping);
        setToolTip(m_textWrapping ? Tr::tr("Disable text wrapping to enable Minimap scrollbar")
                                  : QString());
    }

    QCheckBox *m_enabled;
    QSpinBox *m_width;
    QSpinBox *m_lineCountThresHold;
    QSpinBox *m_alpha;
    QCheckBox *m_centerOnClick;
    QCheckBox *m_showLineTooltip;
    QSpinBox *m_pixelsPerLine;
    bool m_textWrapping;
};

class MinimapSettingsPage : public Core::IOptionsPage
{
public:
    MinimapSettingsPage()
    {
        setId(Constants::MINIMAP_SETTINGS);
        setDisplayName(Tr::tr("Minimap"));
        setCategory(TextEditor::Constants::TEXT_EDITOR_SETTINGS_CATEGORY);
        setWidgetCreator([] { return new MinimapSettingsPageWidget; });
    }
};

MinimapSettings::MinimapSettings(QObject *parent)
    : QObject(parent)
    , m_enabled(true)
    , m_width(Constants::MINIMAP_WIDTH_DEFAULT)
    , m_lineCountThreshold(Constants::MINIMAP_MAX_LINE_COUNT_DEFAULT)
    , m_alpha(Constants::MINIMAP_ALPHA_DEFAULT)
    , m_centerOnClick(Constants::MINIMAP_CENTER_ON_CLICK_DEFAULT)
    , m_showLineTooltip(Constants::MINIMAP_SHOW_LINE_TOOLTIP_DEFAULT)
    , m_pixelsPerLine(Constants::MINIMAP_PIXELS_PER_LINE_DEFAULT)
{
    QTC_ASSERT(!m_instance, return);
    m_instance = this;
    fromMap(Utils::storeFromSettings(Utils::keyFromString(minimapPostFix), Core::ICore::settings()));
    m_settingsPage = new MinimapSettingsPage();
}

MinimapSettings::~MinimapSettings()
{
    m_instance = 0;
}

MinimapSettings *MinimapSettings::instance()
{
    return m_instance;
}

Utils::Store MinimapSettings::toMap() const
{
    Utils::Store map;
    map.insert(enabledKey, m_enabled);
    map.insert(widthKey, m_width);
    map.insert(lineCountThresholdKey, m_lineCountThreshold);
    map.insert(alphaKey, m_alpha);
    map.insert(centerOnClickKey, m_centerOnClick);
    map.insert(showLineTooltipKey, m_showLineTooltip);
    map.insert(pixelsPerLineKey, m_pixelsPerLine);
    return map;
}

void MinimapSettings::fromMap(const Utils::Store &map)
{
    m_enabled = map.value(enabledKey, m_enabled).toBool();
    m_width = map.value(widthKey, m_width).toInt();
    m_lineCountThreshold = map.value(lineCountThresholdKey, m_lineCountThreshold).toInt();
    m_alpha = map.value(alphaKey, m_alpha).toInt();
    m_centerOnClick = map.value(centerOnClickKey, m_centerOnClick).toBool();
    m_showLineTooltip = map.value(showLineTooltipKey, m_showLineTooltip).toBool();
    m_pixelsPerLine = map.value(pixelsPerLineKey, m_pixelsPerLine).toInt();
}

bool MinimapSettings::enabled()
{
    return m_instance->m_enabled;
}

int MinimapSettings::width()
{
    return m_instance->m_width;
}

int MinimapSettings::lineCountThreshold()
{
    return m_instance->m_lineCountThreshold;
}

int MinimapSettings::alpha()
{
    return m_instance->m_alpha;
}

bool MinimapSettings::centerOnClick()
{
    return m_instance->m_centerOnClick;
}

bool MinimapSettings::showLineTooltip()
{
    return m_instance->m_showLineTooltip;
}

int MinimapSettings::pixelsPerLine()
{
    return m_instance->m_pixelsPerLine;
}

void MinimapSettings::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged(enabled);
    }
}

void MinimapSettings::setWidth(int width)
{
    if (m_width != width) {
        m_width = width;
        emit widthChanged(width);
    }
}

void MinimapSettings::setLineCountThreshold(int lineCountThreshold)
{
    if (m_lineCountThreshold != lineCountThreshold) {
        m_lineCountThreshold = lineCountThreshold;
        emit lineCountThresholdChanged(lineCountThreshold);
    }
}

void MinimapSettings::setAlpha(int alpha)
{
    if (m_alpha != alpha) {
        m_alpha = alpha;
        emit alphaChanged(alpha);
    }
}

void MinimapSettings::setCenterOnClick(bool centerOnClick)
{
    if (m_centerOnClick != centerOnClick) {
        m_centerOnClick = centerOnClick;
        emit centerOnClickChanged(centerOnClick);
    }
}

void MinimapSettings::setShowLineTooltip(bool showLineTooltip)
{
    if (m_showLineTooltip != showLineTooltip) {
        m_showLineTooltip = showLineTooltip;
        emit showLineTooltipChanged(showLineTooltip);
    }
}

void MinimapSettings::setPixelsPerLine(int pixelsPerLine)
{
    if (m_pixelsPerLine != pixelsPerLine) {
        m_pixelsPerLine = pixelsPerLine;
        emit pixelsPerLineChanged(pixelsPerLine);
    }
}
} // namespace Internal
} // namespace Minimap
