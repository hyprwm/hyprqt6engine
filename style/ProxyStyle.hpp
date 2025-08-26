#pragma once

#include <QProxyStyle>
#include "common.hpp"

class CHyprProxyStyle : public QProxyStyle, public CHyprStyle::CStyleInstance {
    Q_OBJECT;

  public:
    explicit CHyprProxyStyle();

    void reloadSettings() override {
        ;
    }

    virtual ~CHyprProxyStyle();

    int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const override;
};
