/**
 * AboutDialog.cpp - Contains about dialog impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <UI/AboutDialog.h>

#include <wui/locale/locale.hpp>
#include <wui/theme/theme.hpp>
#include <wui/common/about.hpp>
#include <wui/system/uri_tools.hpp>

#include <Version.h>
#include <License/Grants.h>

#include <boost/nowide/convert.hpp>

#include <resource.h>

namespace Client
{

std::shared_ptr<wui::i_theme> MakeFontTheme(int32_t size, wui::decorations decorations)
{
    auto appNameTheme = wui::make_custom_theme();

    appNameTheme->load_theme(*wui::get_default_theme());

    auto font = appNameTheme->get_font(wui::text::tc, wui::text::tv_font);
    font.size = size;
    font.decorations_ = decorations;
    appNameTheme->set_font(wui::text::tc, wui::text::tv_font, font);

    return appNameTheme;
}

AboutDialog::AboutDialog(std::weak_ptr<wui::window> transientWindow_)
    : transientWindow(transientWindow_),
    window(new wui::window()),
    logo(),
    appName(), client(),
    version(),
    license(),
    wuiInfo(),
    wuiURLAnchor(),
    vendorURLAnchor(),
    copyright(),
    closeButton()
{
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::Run(uint32_t grants_)
{
    auto appNameFont = MakeFontTheme(42, wui::decorations::bold);
    auto mediumFont = MakeFontTheme(24, wui::decorations::normal);

    License::Grants grants = License::Parse(grants_);

    window->set_transient_for(transientWindow.lock());
    logo = std::shared_ptr<wui::image>(new wui::image(IMG_LOGO));
    appName = std::shared_ptr<wui::text>(new wui::text(wui::locale("about", "app_name"), wui::hori_alignment::center));
    appName->update_theme(appNameFont);
    client = std::shared_ptr<wui::text>(new wui::text(wui::locale("about", "client"), wui::hori_alignment::center));
    client->update_theme(mediumFont);
    version = std::shared_ptr<wui::text>(new wui::text(SYSTEM_VERSION, wui::hori_alignment::center));
    version->update_theme(mediumFont);
    license = std::shared_ptr<wui::text>(new wui::text(wui::locale("about", grants.freeLicense ? "license_free" : "license_commercial"), wui::hori_alignment::center));
    wuiInfo = std::shared_ptr<wui::text>(new wui::text(wui::about::full_name + std::string("\n") + wui::about::version, wui::hori_alignment::center));
    wuiURLAnchor= std::shared_ptr<wui::button>(new wui::button(wui::about::web, [](){ wui::open_uri(wui::about::web); }, wui::button_view::anchor));
    vendorURLAnchor = std::shared_ptr<wui::button>(new wui::button("www." + wui::locale("about", "vendor_url"), [](){ wui::open_uri(std::string("https://") + wui::locale("about", "vendor_url")); }, wui::button_view::anchor));
    copyright = std::shared_ptr<wui::text>(new wui::text(wui::locale("about", "copyright"), wui::hori_alignment::center));
    closeButton = std::shared_ptr<wui::button>(new wui::button(wui::locale("button", "close"), [this]() { window->destroy(); }, "green_button"));

    window->add_control(logo, { 100, 30, 300, 230 });
    window->add_control(appName, { 10, 235, WND_WIDTH - 10, 255 });
    window->add_control(client, { 10, 280, WND_WIDTH - 10, 295 });
    window->add_control(version, { 10, 315, WND_WIDTH - 10, 330 });
    window->add_control(license, { 10, 360, WND_WIDTH - 10, 375 });

    window->add_control(wuiInfo, { 10, 400, WND_WIDTH - 10, 430 });
    window->add_control(wuiURLAnchor, { 150, 430, 320, 445 });

    window->add_control(vendorURLAnchor, { 145, WND_HEIGHT - 95, 280, WND_HEIGHT - 70 });
    window->add_control(copyright, { 10, WND_HEIGHT - 70, WND_WIDTH - 10, WND_HEIGHT - 55 });
    window->add_control(closeButton, { WND_WIDTH - 110, WND_HEIGHT - 40, WND_WIDTH - 10, WND_HEIGHT - 10 });

    window->set_default_push_control(closeButton);

    window->init(wui::locale("about", "title"), { -1, -1, WND_WIDTH, WND_HEIGHT }, wui::window_style::dialog, [this]() {
        closeButton.reset();
        copyright.reset();
        vendorURLAnchor.reset();
        wuiURLAnchor.reset();
        wuiInfo.reset();
        license.reset();
        version.reset();
        client.reset();
        appName.reset();
        logo.reset();
    });
}

}
