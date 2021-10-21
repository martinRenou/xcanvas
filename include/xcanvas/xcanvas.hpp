/******************************************************************************
* Copyright (c) 2021, Martin Renou                                            *
*                                                                             *
* Distributed under the terms of the BSD 3-Clause License.                    *
*                                                                             *
* The full license is in the file LICENSE, distributed with this software.    *
*******************************************************************************/

#ifndef XCANVAS_HPP
#define XCANVAS_HPP

#include <array>
#include <functional>
#include <list>
#include <string>
#include <utility>
#include <vector>

#include "xtl/xoptional.hpp"

#include "nlohmann/json.hpp"

#include "xwidgets/xcolor.hpp"
#include "xwidgets/xeither.hpp"
#include "xwidgets/xholder.hpp"
#include "xwidgets/xmaterialize.hpp"
#include "xwidgets/xobject.hpp"
#include "xwidgets/xstyle.hpp"
#include "xwidgets/xwidget.hpp"

#include "xcanvas_config.hpp"

namespace nl = nlohmann;


namespace xc
{
    enum COMMANDS {
        fillRect, strokeRect, fillRects, strokeRects, clearRect, fillArc,
        fillCircle, strokeArc, strokeCircle, fillArcs, strokeArcs,
        fillCircles, strokeCircles, strokeLine, beginPath, closePath,
        stroke, fillPath, fill, moveTo, lineTo,
        rect, arc, ellipse, arcTo, quadraticCurveTo,
        bezierCurveTo, fillText, strokeText, setLineDash, drawImage,
        putImageData, clip, save, restore, translate,
        rotate, scale, transform, setTransform, resetTransform,
        set, clear, sleep, fillPolygon, strokePolygon,
        strokeLines
    };

    /**********************
     * canvas declaration *
     **********************/

    template <class D>
    class xcanvas : public xw::xwidget<D>
    {
    public:

        using base_type = xw::xwidget<D>;
        using derived_type = D;

        void serialize_state(nl::json&, xeus::buffer_sequence&) const;
        void apply_patch(const nl::json&, const xeus::buffer_sequence&);

        XPROPERTY(int, derived_type, width, 700);
        XPROPERTY(int, derived_type, height, 500);
        XPROPERTY(bool, derived_type, sync_image_data, false);

        void fill_rect(int x, int y, int width);
        void fill_rect(int x, int y, int width, int height);

        void flush();

        void handle_custom_message(const nl::json&);

    protected:

        xcanvas();
        using base_type::base_type;

    private:

        void send_command(const nl::json& command);

        nl::json m_commands;
        bool m_caching;
    };

    using canvas = xw::xmaterialize<xcanvas>;

    /**************************
     * xcanvas implementation *
     **************************/

    template <class D>
    inline xcanvas<D>::xcanvas()
        : base_type()
    {
        this->_model_module() = "ipycanvas";
        this->_view_module() = "ipycanvas";
        this->_model_name() = "CanvasModel";
        this->_view_name() = "CanvasView";
        this->_model_module_version() = jupyter_canvas_semver();
        this->_view_module_version() = jupyter_canvas_semver();

        m_commands = nl::json::array();
        m_caching = false;
    }

    template <class D>
    inline void xcanvas<D>::serialize_state(nl::json& state, xeus::buffer_sequence& buffers) const
    {
        base_type::serialize_state(state, buffers);

        using xw::xwidgets_serialize;

        xwidgets_serialize(width(), state["width"], buffers);
        xwidgets_serialize(height(), state["height"], buffers);
        xwidgets_serialize(sync_image_data(), state["sync_image_data"], buffers);
    }

    template <class D>
    inline void xcanvas<D>::apply_patch(const nl::json& patch, const xeus::buffer_sequence& buffers)
    {
        base_type::apply_patch(patch, buffers);

        using xw::set_property_from_patch;

        set_property_from_patch(width, patch, buffers);
        set_property_from_patch(height, patch, buffers);
        set_property_from_patch(sync_image_data, patch, buffers);
    }

    template <class D>
    inline void xcanvas<D>::fill_rect(int x, int y, int width)
    {
        send_command(nl::json::array({ COMMANDS::fillRect, { x, y, width, width } }));
    }

    template <class D>
    inline void xcanvas<D>::fill_rect(int x, int y, int width, int height)
    {
        send_command(nl::json::array({ COMMANDS::fillRect, { x, y, width, height } }));
    }

    template <class D>
    inline void xcanvas<D>::send_command(const nl::json& command)
    {
        m_commands.push_back(command);

        if (!m_caching)
        {
            flush();
        }
    }

    template <class D>
    inline void xcanvas<D>::flush()
    {
        nl::json content;
        content["dtype"] = "uint8";

        std::string buffer_str = m_commands.dump();
        std::vector<char> buffer(buffer_str.begin(), buffer_str.end());

        this->send(std::move(content), { buffer });

        m_commands.clear();
    }

    template <class D>
    inline void xcanvas<D>::handle_custom_message(const nl::json& content)
    {
        // auto it = content.find("event");
        // if (it != content.end() && it.value() == "interaction")
        // {
        //     for (auto it = m_interaction_callbacks.begin(); it != m_interaction_callbacks.end(); ++it)
        //     {
        //         it->operator()(content);
        //     }
        // }
    }
}

/*********************
 * precompiled types *
 *********************/

    extern template class xw::xmaterialize<xc::xcanvas>;
    extern template class xw::xtransport<xw::xmaterialize<xc::xcanvas>>;

#endif
