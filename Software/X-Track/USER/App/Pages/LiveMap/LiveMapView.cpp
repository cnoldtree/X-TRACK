#include "LiveMapView.h"
#include <stdarg.h>
#include <stdio.h>

using namespace Page;

void LiveMapView::Create(lv_obj_t* root, uint32_t tileNum)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    lv_obj_t* label = lv_label_create(root);
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, Resource.GetFont("bahnschrift_17"), 0);
    lv_label_set_text(label, "LOADING...");
    ui.labelInfo = label;

    Style_Create();
    Map_Create(root, tileNum);
    ZoomCtrl_Create(root);
    SportInfo_Create(root);
}

void LiveMapView::Delete()
{
    if (ui.map.imgTiles)
    {
        lv_mem_free(ui.map.imgTiles);
        ui.map.imgTiles = nullptr;
    }

    std::vector<lv_point_t, lv_allocator<lv_point_t>> vec;
    ui.map.trackPoints.swap(vec);

    lv_style_reset(&ui.styleCont);
    lv_style_reset(&ui.styleLabel);
    lv_style_reset(&ui.styleLine);
}

void LiveMapView::Style_Create()
{
    lv_style_init(&ui.styleCont);
    lv_style_set_bg_color(&ui.styleCont, lv_color_black());
    lv_style_set_bg_opa(&ui.styleCont, LV_OPA_60);
    lv_style_set_radius(&ui.styleCont, 6);
    lv_style_set_shadow_width(&ui.styleCont, 10);
    lv_style_set_shadow_color(&ui.styleCont, lv_color_black());

    lv_style_init(&ui.styleLabel);
    lv_style_set_text_font(&ui.styleLabel, Resource.GetFont("bahnschrift_17"));
    lv_style_set_text_color(&ui.styleLabel, lv_color_white());

    lv_style_init(&ui.styleLine);
    lv_style_set_line_color(&ui.styleLine, lv_color_hex(0xff931e));
    lv_style_set_line_width(&ui.styleLine, 5);
    lv_style_set_line_opa(&ui.styleLine, LV_OPA_COVER);
    lv_style_set_line_rounded(&ui.styleLine, true);
}

void LiveMapView::Map_Create(lv_obj_t* par, uint32_t tileNum)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
#if 0
    lv_obj_set_style_outline_color(cont, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_outline_width(cont, 2, 0);
#endif
    ui.map.cont = cont;

    ui.map.imgTiles = (lv_obj_t**)lv_mem_alloc(tileNum * sizeof(lv_obj_t*));
    ui.map.tileNum = tileNum;

    for (uint32_t i = 0; i < tileNum; i++)
    {
        lv_obj_t* img = lv_img_create(cont);
        lv_obj_remove_style_all(img);
        ui.map.imgTiles[i] = img;
    }

    TrackLine_Create(cont);

    lv_obj_t* img = lv_img_create(cont);
    lv_img_set_src(img, Resource.GetImage("gps_arrow"));

    lv_img_t* imgOri = (lv_img_t*)img;
    lv_obj_set_pos(img, -imgOri->w, -imgOri->h);
    ui.map.imgArrow = img;
}

void LiveMapView::SetMapTile(uint32_t tileSize, uint32_t widthCnt)
{
    uint32_t tileNum = ui.map.tileNum;

    lv_coord_t width = (lv_coord_t)(tileSize * widthCnt);
    lv_coord_t height = (lv_coord_t)(tileSize * (ui.map.tileNum / widthCnt));

    lv_obj_set_size(ui.map.cont, width, height);

    for (uint32_t i = 0; i < tileNum; i++)
    {
        lv_obj_t* img = ui.map.imgTiles[i];

        lv_obj_set_size(img, tileSize, tileSize);

        lv_coord_t x = (i % widthCnt) * tileSize;
        lv_coord_t y = (i / widthCnt) * tileSize;
        lv_obj_set_pos(img, x, y);
    }
}

void LiveMapView::ZoomCtrl_Create(lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_add_style(cont, &ui.styleCont, 0);
    lv_obj_set_style_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_size(cont, 50, 30);
    lv_obj_set_pos(cont, lv_obj_get_style_width(par, 0) - lv_obj_get_style_width(cont, 0) + 5, 40);
    ui.zoom.cont = cont;

    static const lv_style_prop_t prop[] =
    {
        LV_STYLE_X,
        LV_STYLE_OPA,
        LV_STYLE_PROP_INV
    };
    static lv_style_transition_dsc_t tran;
    lv_style_transition_dsc_init(&tran, prop, lv_anim_path_ease_out, 200, 0, nullptr);
    lv_obj_set_style_x(cont, lv_obj_get_style_width(par, 0), LV_STATE_USER_1);
    lv_obj_set_style_opa(cont, LV_OPA_TRANSP, LV_STATE_USER_1);
    lv_obj_add_state(cont, LV_STATE_USER_1);
    lv_obj_set_style_transition(cont, &tran, LV_STATE_USER_1);
    lv_obj_set_style_transition(cont, &tran, 0);


    lv_obj_t* label = lv_label_create(cont);
    lv_obj_add_style(label, &ui.styleLabel, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, -2, 0);
    lv_label_set_text(label, "--");
    ui.zoom.labelInfo = label;

    lv_obj_t* slider = lv_slider_create(cont);
    lv_obj_remove_style_all(slider);
    lv_slider_set_value(slider, 15, LV_ANIM_OFF);
    ui.zoom.slider = slider;
}

void LiveMapView::SportInfo_Create(lv_obj_t* par)
{
    /* cont */
    lv_obj_t* obj = lv_obj_create(par);
    lv_obj_remove_style_all(obj);
    lv_obj_add_style(obj, &ui.styleCont, 0);
    lv_obj_set_size(obj, 159, 66);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_LEFT, -10, 10);
    lv_obj_set_style_radius(obj, 10, 0);
    ui.sportInfo.cont = obj;

    /* speed */
    lv_obj_t* label = lv_label_create(obj);
    lv_label_set_text(label, "00");
    lv_obj_set_style_text_font(label, Resource.GetFont("bahnschrift_32"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 20, -10);
    ui.sportInfo.labelSpeed = label;

    label = lv_label_create(obj);
    lv_label_set_text(label, "km/h");
    lv_obj_set_style_text_font(label, Resource.GetFont("bahnschrift_13"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align_to(label, ui.sportInfo.labelSpeed, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);

    ui.sportInfo.labelTrip = ImgLabel_Create(obj, Resource.GetImage("trip"), 5, 10);
    ui.sportInfo.labelTime = ImgLabel_Create(obj, Resource.GetImage("alarm"), 5, 30);
}

lv_obj_t* LiveMapView::ImgLabel_Create(lv_obj_t* par, const void* img_src, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    lv_obj_t* img = lv_img_create(par);
    lv_img_set_src(img, img_src);

    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, y_ofs);

    lv_obj_t* label = lv_label_create(par);
    lv_label_set_text(label, "--");
    lv_obj_add_style(label, &ui.styleLabel, 0);
    lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, x_ofs, 0);
    return label;
}

void LiveMapView::TrackLine_Create(lv_obj_t* par)
{
    lv_obj_t* line = lv_line_create(par);
    lv_obj_remove_style_all(line);
    lv_obj_add_style(line, &ui.styleLine, 0);

    ui.map.lineTrack = line;

    line = lv_line_create(par);
    lv_obj_remove_style_all(line);
    lv_obj_add_style(line, &ui.styleLine, 0);

    ui.map.lineActive = line;
}

void LiveMapView::TrackAddPoint(lv_coord_t x, lv_coord_t y)
{
    lv_point_t point = { x, y };
    ui.map.trackPoints.push_back(point);
}

void LiveMapView::TrackReset()
{
    ui.map.trackPoints.clear();
    lv_line_set_points(ui.map.lineTrack, nullptr, 0);
    lv_line_set_points(ui.map.lineActive, nullptr, 0);
}

void LiveMapView::TrackSetActivePoint(lv_coord_t x, lv_coord_t y)
{
    if (ui.map.trackPoints.size())
    {
        lv_line_set_points(
            ui.map.lineTrack,
            &ui.map.trackPoints[0],
            (uint16_t)ui.map.trackPoints.size()
        );
    }

    lv_point_t end = ui.map.trackPoints.back();
    ui.map.pointActive[0] = end;
    ui.map.pointActive[1].x = x;
    ui.map.pointActive[1].y = y;
    lv_line_set_points(ui.map.lineActive, ui.map.pointActive, 2);
}
