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

#include "minimapstyle.h"
#include "minimapconstants.h"
#include "minimapsettings.h"

#include <texteditor/displaysettings.h>
#include <texteditor/fontsettings.h>
#include <texteditor/tabsettings.h>
#include <texteditor/textdocument.h>
#include <texteditor/textdocumentlayout.h>
#include <texteditor/texteditor.h>
#include <texteditor/texteditorsettings.h>
#include <utils/theme/theme.h>

#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOption>
#include <QTextBlock>
#include <QTextDocument>
#include <QTimer>

#include <algorithm>

namespace Minimap {
namespace Internal {
namespace {
const QRgb black = QColor(Qt::black).rgb();
const QRgb red = QColor(Qt::red).rgb();
const QRgb green = QColor(Qt::darkGreen).rgb();

inline QColor blendColors(const QColor &a, const QColor &b)
{
    int c = qMin(255, a.cyan() + b.cyan());
    int m = qMin(255, a.magenta() + b.magenta());
    int y = qMin(255, a.yellow() + b.yellow());
    int k = qMin(255, a.black() + b.black());
    return QColor::fromCmyk(c, m, y, k);
}

inline bool updatePixel(QRgb *scanLine,
                        bool blend,
                        const QChar &c,
                        int &x,
                        int w,
                        int tab,
                        const QColor &bg,
                        const QColor &fg)
{
    if (c == QChar::Tabulation) {
        for (int i = 0; i < tab; ++i) {
            if (!blend) {
                scanLine[x++] = bg.rgb();
            }
            if (x >= w) {
                return false;
            }
        }
    } else {
        bool isSpace = c.isSpace();
        if (blend && !isSpace) {
            QColor result = blendColors(fg.toCmyk(), QColor(scanLine[x]).toCmyk()).toRgb();
            scanLine[x++] = result.rgb();
        } else {
            scanLine[x++] = isSpace ? bg.rgb() : fg.rgb();
        }
        if (x >= w) {
            return false;
        }
    }
    return true;
}

inline void merge(QColor &bg, QColor &fg, const QTextCharFormat &f)
{
    if (f.background().style() != Qt::NoBrush) {
        bg = f.background().color();
    }
    if (f.foreground().style() != Qt::NoBrush) {
        fg = f.foreground().color();
    }
}
} // namespace

class MinimapStyleObject : public QObject
{
public:
    MinimapStyleObject(TextEditor::BaseTextEditor *editor)
        : QObject(editor->editorWidget())
        , m_theme(Utils::creatorTheme())
        , m_editor(editor->editorWidget())
        , m_update(false)
    {
        m_editor->installEventFilter(this);
        if (!m_editor->textDocument()->document()->isEmpty()) {
            init();
        } else {
            connect(m_editor->textDocument()->document(),
                    &QTextDocument::contentsChanged,
                    this,
                    &MinimapStyleObject::contentsChanged);
        }
    }

    ~MinimapStyleObject() { m_editor->removeEventFilter(this); }

    bool eventFilter(QObject *watched, QEvent *event)
    {
        if (watched == m_editor && event->type() == QEvent::Resize) {
            deferedUpdate();
        }
        return false;
    }

    int width() const
    {
        int w = m_editor->extraArea() ? m_editor->extraArea()->width() : 0;
        return qMin(m_editor->width() - w,
                    MinimapSettings::width() + Constants::MINIMAP_EXTRA_AREA_WIDTH);
    }

    const QRect &groove() const { return m_groove; }

    const QRect &addPage() const { return m_addPage; }

    const QRect &subPage() const { return m_subPage; }

    const QRect &slider() const { return m_slider; }

    int lineCount() const { return m_lineCount; }

    qreal factor() const { return m_factor; }

    const QColor &background() const { return m_backgroundColor; }

    const QColor &foreground() const { return m_foregroundColor; }

    const QColor &overlay() const { return m_overlayColor; }

    TextEditor::TextEditorWidget *editor() const { return m_editor; }

private:
    void init()
    {
        QScrollBar *scrollbar = m_editor->verticalScrollBar();
        scrollbar->setProperty(Constants::MINIMAP_STYLE_OBJECT_PROPERTY,
                               QVariant::fromValue<QObject *>(this));
        connect(m_editor->textDocument(),
                &TextEditor::TextDocument::fontSettingsChanged,
                this,
                &MinimapStyleObject::fontSettingsChanged);
        connect(m_editor->document()->documentLayout(),
                &QAbstractTextDocumentLayout::documentSizeChanged,
                this,
                &MinimapStyleObject::deferedUpdate);
        connect(m_editor->document()->documentLayout(),
                &QAbstractTextDocumentLayout::update,
                this,
                &MinimapStyleObject::deferedUpdate);
        connect(MinimapSettings::instance(),
                &MinimapSettings::enabledChanged,
                this,
                &MinimapStyleObject::deferedUpdate);
        connect(MinimapSettings::instance(),
                &MinimapSettings::widthChanged,
                this,
                &MinimapStyleObject::deferedUpdate);
        connect(MinimapSettings::instance(),
                &MinimapSettings::lineCountThresholdChanged,
                this,
                &MinimapStyleObject::deferedUpdate);
        connect(MinimapSettings::instance(),
                &MinimapSettings::alphaChanged,
                this,
                &MinimapStyleObject::fontSettingsChanged);
        connect(scrollbar,
                &QAbstractSlider::valueChanged,
                this,
                &MinimapStyleObject::updateSubControlRects);
        fontSettingsChanged();
    }

    void contentsChanged()
    {
        disconnect(m_editor->textDocument()->document(),
                   &QTextDocument::contentsChanged,
                   this,
                   &MinimapStyleObject::contentsChanged);
        init();
    }

    void fontSettingsChanged()
    {
        const TextEditor::FontSettings &settings = m_editor->textDocument()->fontSettings();
        m_backgroundColor = settings.formatFor(TextEditor::C_TEXT).background();
        if (!m_backgroundColor.isValid()) {
            m_backgroundColor = m_theme->color(Utils::Theme::BackgroundColorNormal);
        }
        m_foregroundColor = settings.formatFor(TextEditor::C_TEXT).foreground();
        if (!m_foregroundColor.isValid()) {
            m_foregroundColor = m_theme->color(Utils::Theme::TextColorNormal);
        }
        if (m_backgroundColor.value() < 128) {
            m_overlayColor = QColor(Qt::white);
        } else {
            m_overlayColor = QColor(Qt::black);
        }
        m_overlayColor.setAlpha(MinimapSettings::alpha());
        deferedUpdate();
    }

    void deferedUpdate()
    {
        if (m_update) {
            return;
        }
        m_update = true;
        QTimer::singleShot(0, this, &MinimapStyleObject::update);
    }

    void update()
    {
        QScrollBar *scrollbar = m_editor->verticalScrollBar();
        QSizeF sz = m_editor->document()->size();
        m_lineCount = sz.toSize().height() + 1;
        int w = scrollbar->width();
        int h = scrollbar->height();
        m_factor = m_lineCount <= h ? 1.0 : h / static_cast<qreal>(m_lineCount);
        int width = this->width();
        m_groove = QRect(width, 0, w - width, qMin(m_lineCount, h));
        updateSubControlRects();
        scrollbar->updateGeometry();
        m_update = false;
    }

    void updateSubControlRects()
    {
        QScrollBar *scrollbar = m_editor->verticalScrollBar();
        int viewPortLineCount = qRound(m_factor * (m_lineCount - scrollbar->maximum()));
        int w = scrollbar->width();
        int h = scrollbar->height();
        int value = scrollbar->value();
        int realValue = qRound(m_factor * value);
        int min = scrollbar->minimum();
        int max = scrollbar->maximum();
        m_addPage = value < max ? QRect(0,
                                        realValue + viewPortLineCount,
                                        w,
                                        h - viewPortLineCount - realValue)
                                : QRect();
        m_subPage = value > min ? QRect(0, 0, w, realValue) : QRect();
        m_slider = QRect(0, realValue, w, viewPortLineCount);
        scrollbar->update();
    }

    Utils::Theme *m_theme;
    TextEditor::TextEditorWidget *m_editor;
    qreal m_factor;
    int m_lineCount;
    QRect m_groove, m_addPage, m_subPage, m_slider;
    QColor m_backgroundColor, m_foregroundColor, m_overlayColor;
    bool m_update;
};

MinimapStyle::MinimapStyle(QStyle *style)
    : QProxyStyle(style)
{}

void MinimapStyle::drawComplexControl(ComplexControl control,
                                      const QStyleOptionComplex *option,
                                      QPainter *painter,
                                      const QWidget *widget) const
{
    if (widget && control == QStyle::CC_ScrollBar && MinimapSettings::enabled()) {
        QVariant v = widget->property(Constants::MINIMAP_STYLE_OBJECT_PROPERTY);
        if (v.isValid()) {
            MinimapStyleObject *o = static_cast<MinimapStyleObject *>(v.value<QObject *>());
            int lineCount = o->lineCount();
            if (lineCount > 0 && lineCount <= MinimapSettings::lineCountThreshold()) {
                if (drawMinimap(option, painter, widget, o)) {
                    return;
                }
            }
        }
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

QStyle::SubControl MinimapStyle::hitTestComplexControl(ComplexControl control,
                                                       const QStyleOptionComplex *option,
                                                       const QPoint &pos,
                                                       const QWidget *widget) const
{
    if (widget && control == QStyle::CC_ScrollBar && MinimapSettings::enabled()) {
        QVariant v = widget->property(Constants::MINIMAP_STYLE_OBJECT_PROPERTY);
        if (v.isValid()) {
            MinimapStyleObject *o = static_cast<MinimapStyleObject *>(v.value<QObject *>());
            int lineCount = o->lineCount();
            if (lineCount > 0 && lineCount <= MinimapSettings::lineCountThreshold()) {
                SubControl sc = SC_None;
                if (const QStyleOptionSlider *scrollbar
                    = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
                    QRect r;
                    uint ctrl = SC_ScrollBarAddLine;
                    while (ctrl <= SC_ScrollBarGroove) {
                        r = subControlRect(control, scrollbar, QStyle::SubControl(ctrl), widget);
                        if (r.isValid() && r.contains(pos)) {
                            sc = QStyle::SubControl(ctrl);
                            break;
                        }
                        ctrl <<= 1;
                    }
                }
                return sc;
            }
        }
    }
    return QProxyStyle::hitTestComplexControl(control, option, pos, widget);
}

int MinimapStyle::pixelMetric(PixelMetric metric,
                              const QStyleOption *option,
                              const QWidget *widget) const
{
    if (widget && metric == QStyle::PM_ScrollBarExtent && MinimapSettings::enabled()) {
        int w = QProxyStyle::pixelMetric(metric, option, widget);
        QVariant v = widget->property(Constants::MINIMAP_STYLE_OBJECT_PROPERTY);
        if (v.isValid()) {
            MinimapStyleObject *o = static_cast<MinimapStyleObject *>(v.value<QObject *>());
            int lineCount = o->lineCount();
            if (lineCount > 0 && lineCount <= MinimapSettings::lineCountThreshold()) {
                w += o->width();
            }
        }
        return w;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QRect MinimapStyle::subControlRect(ComplexControl cc,
                                   const QStyleOptionComplex *opt,
                                   SubControl sc,
                                   const QWidget *widget) const
{
    if (widget && cc == QStyle::CC_ScrollBar && MinimapSettings::enabled()) {
        QVariant v = widget->property(Constants::MINIMAP_STYLE_OBJECT_PROPERTY);
        if (v.isValid()) {
            MinimapStyleObject *o = static_cast<MinimapStyleObject *>(v.value<QObject *>());
            int lineCount = o->lineCount();
            if (lineCount > 0 && lineCount <= MinimapSettings::lineCountThreshold()) {
                switch (sc) {
                case QStyle::SC_ScrollBarGroove:
                    return o->groove();
                case QStyle::SC_ScrollBarAddPage:
                    return o->addPage();
                case QStyle::SC_ScrollBarSubPage:
                    return o->subPage();
                case QStyle::SC_ScrollBarSlider:
                    return o->slider();
                default:
                    return QRect();
                }
            }
        }
    }
    return QProxyStyle::subControlRect(cc, opt, sc, widget);
}

bool MinimapStyle::drawMinimap(const QStyleOptionComplex *option,
                               QPainter *painter,
                               const QWidget *widget,
                               MinimapStyleObject *o) const
{
    if (TextEditor::TextEditorSettings::displaySettings().m_textWrapping) {
        return false;
    }
    const QScrollBar *scrollbar = qobject_cast<const QScrollBar *>(widget);
    if (!scrollbar) {
        return false;
    }
    int h = o->editor()->size().height();
    qreal factor = o->factor();
    if (factor < 1.0) {
        h = o->lineCount();
    }
    qreal step = 1 / factor;
    QColor baseBg = o->background();
    QColor baseFg = o->foreground();
    int w = o->width() - Constants::MINIMAP_EXTRA_AREA_WIDTH;
    QImage image(o->width(), h, QImage::Format_RGB32);
    image.fill(baseBg);
    QTextDocument *doc = o->editor()->document();
    TextEditor::TextDocumentLayout *documentLayout = qobject_cast<TextEditor::TextDocumentLayout *>(
        doc->documentLayout());
    int tab = o->editor()->textDocument()->tabSettings().m_tabSize;
    int y(0);
    int i(0);
    qreal r(0.0);
    bool codeFoldingVisible = o->editor()->codeFoldingVisible();
    bool revisionsVisible = o->editor()->revisionsVisible();
    bool folded(false);
    int revision(0);
    for (QTextBlock b = doc->begin(); b.isValid() && y < h; b = b.next()) {
        bool updateY(true);
        if (b.isVisible()) {
            if (qRound(r) != i++) {
                updateY = false;
            } else {
                r += step;
            }
        } else {
            continue;
        }
        if (codeFoldingVisible && !folded) {
            folded = TextEditor::TextDocumentLayout::isFolded(b);
        }
        if (revisionsVisible) {
            if (b.revision() != documentLayout->lastSaveRevision) {
                if (revision < 1 && b.revision() < 0) {
                    revision = 1;
                } else if (revision < 2) {
                    revision = 2;
                }
            }
        }
        int x(0);
        bool cont(true);
        QRgb *scanLine = reinterpret_cast<QRgb *>(image.scanLine(y));
        QVector<QTextLayout::FormatRange> formats = b.layout()->formats();
        std::sort(formats.begin(),
                  formats.end(),
                  [](const QTextLayout::FormatRange &r1, const QTextLayout::FormatRange &r2) {
                      if (r1.start < r2.start) {
                          return true;
                      } else if (r1.start > r2.start) {
                          return false;
                      }
                      return r1.length < r2.length;
                  });
        QColor bBg = baseBg;
        QColor bFg = baseFg;
        merge(bBg, bFg, b.charFormat());
        auto it2 = formats.begin();
        for (QTextBlock::iterator it = b.begin(); !(it.atEnd()); ++it) {
            QTextFragment f = it.fragment();
            if (f.isValid()) {
                QColor fBg = bBg;
                QColor fFg = bFg;
                merge(fBg, fFg, f.charFormat());
                for (const QChar &c : f.text()) {
                    QColor bg = fBg;
                    QColor fg = fFg;
                    it2 = std::find_if(it2, formats.end(), [&x](const QTextLayout::FormatRange &r) {
                        return x >= r.start && x < r.start + r.length;
                    });
                    if (it2 != formats.end()) {
                        merge(bg, fg, it2->format);
                    }
                    cont = updatePixel(&scanLine[Constants::MINIMAP_EXTRA_AREA_WIDTH],
                                       !updateY,
                                       c,
                                       x,
                                       w,
                                       tab,
                                       bg,
                                       fg);
                    if (!cont) {
                        break;
                    }
                }
                if (!cont) {
                    break;
                }
            } else {
                cont = false;
                break;
            }
        }
        if (updateY) {
            ++y;
            if (revision == 1) {
                scanLine[1] = green;
                scanLine[2] = green;
            } else if (revision == 2) {
                scanLine[1] = red;
                scanLine[2] = red;
            }
            if (folded) {
                scanLine[4] = black;
                scanLine[5] = black;
            }
            folded = false;
            revision = 0;
        }
    }
    painter->save();
    painter->fillRect(option->rect, baseBg);
    painter->drawImage(option->rect, image, option->rect);
    painter->setPen(Qt::NoPen);
    painter->setBrush(o->overlay());
    QRect rect = subControlRect(QStyle::CC_ScrollBar, option, QStyle::SC_ScrollBarSlider, widget)
                     .intersected(option->rect);
    painter->drawRect(rect);
    painter->restore();
    return true;
}

QObject *MinimapStyle::createMinimapStyleObject(TextEditor::BaseTextEditor *editor)
{
    return new MinimapStyleObject(editor);
}
} // namespace Internal
} // namespace Minimap
