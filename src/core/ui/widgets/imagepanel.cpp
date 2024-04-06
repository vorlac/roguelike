#include "core/assert.hpp"
#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/widgets/imagepanel.hpp"

namespace rl::ui {

    ImagePanel::ImagePanel(Widget* parent)
        : Widget(parent)
    {
    }

    void ImagePanel::set_images(const ImagePanel::Images& data)
    {
        m_images = data;
    }

    const ImagePanel::Images& ImagePanel::images() const
    {
        return m_images;
    }

    const std::function<void(int)>& ImagePanel::callback() const
    {
        return m_callback;
    }

    void ImagePanel::set_callback(const std::function<void(int)>& callback)
    {
        m_callback = callback;
    }

    ds::dims<i32> ImagePanel::grid_size() const
    {
        const i32 cols{ 1 + std::max(0, static_cast<i32>((m_rect.size.width - 2.0f * m_margin.x -
                                                          m_thumb_size.width) /
                                                         (m_thumb_size.width + m_spacing.x))) };
        const i32 rows{ (static_cast<i32>(m_images.size()) + cols - 1) / cols };
        return ds::dims{ cols, rows };
    }

    i32 ImagePanel::index_for_position(const ds::point<f32>& mouse_pos) const
    {
        // TODO: revisit this and remove the duplicate logic if doesn't trigger assert
        const ds::point<f32> pp{ ((mouse_pos - m_rect.pt) - m_margin) / (m_thumb_size + m_spacing) };
        const ds::dims<f32> icon_size{ m_thumb_size / ds::dims<f32>{ m_thumb_size + m_spacing } };
        const ds::rect<f32> image_rect{ pp, icon_size };

        bool over_image{ image_rect.contains(mouse_pos) };
        bool over_image2{ pp.x - std::floor(pp.x) < icon_size.width &&
                          pp.y - std::floor(pp.y) < icon_size.height };

        runtime_assert(over_image == over_image2,
                       "image overlap inconsistency\nover_image={}\nover_image2={}", over_image,
                       over_image2);

        const ds::rect<f32> grid_rect{
            mouse_pos,
            this->grid_size(),
        };

        over_image &= grid_rect.contains(pp);
        over_image2 &= mouse_pos.x >= 0 && mouse_pos.y >= 0 && pp.x >= 0 && pp.y >= 0 &&
                       mouse_pos.x < static_cast<f32>(this->grid_size().width) &&
                       mouse_pos.y < static_cast<f32>(this->grid_size().height);

        runtime_assert(over_image == over_image2,
                       "image overlap inconsistency\nover_image={}\nover_image2={}", over_image,
                       over_image2);

        return over_image
                 ? static_cast<i32>(
                       mouse_pos.x + mouse_pos.y * static_cast<f32>(this->grid_size().width))
                 : -1;
    }

    bool ImagePanel::on_mouse_move(const Mouse& mouse, const Keyboard& kb)
    {
        m_mouse_index = this->index_for_position(mouse.pos());
        return true;
    }

    bool ImagePanel::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        const i32 index{ this->index_for_position(mouse.pos()) };
        if (index >= 0 && index < static_cast<i32>(m_images.size()) && m_callback != nullptr)
            m_callback(index);
        return true;
    }

    ds::dims<f32> ImagePanel::preferred_size() const
    {
        ds::dims<f32> grid{ static_cast<ds::dims<f32>>(this->grid_size()) };
        return ds::dims<f32>{
            (m_thumb_size.width * grid.width) + ((grid.width - 1.0f) * m_spacing.x) +
                (m_margin.x * 2.0f),
            (m_thumb_size.height * grid.height) + ((grid.height - 1.0f) * m_spacing.y) +
                (m_margin.y * 2.0f),
        };
    }

    void ImagePanel::draw()
    {
        const ds::dims<i32> grid{ this->grid_size() };
        auto&& context{ m_renderer->context() };

        for (auto i = 0; i < m_images.size(); ++i)
        {
            const ds::point<f32> p{
                m_rect.pt + m_margin +
                    ds::dims<f32>{ m_thumb_size + m_spacing } *
                        ds::vector2{ i % grid.width, i / grid.height },
            };

            ds::dims<f32> image_size{ 0.0f, 0.0f };
            nvg::image_size(context, m_images[i].first, &image_size.width, &image_size.height);

            ds::rect<f32> image_rect{ 0, 0, 0, 0 };
            if (image_size.width < image_size.height)
            {
                image_rect.size.width = m_thumb_size.width;
                image_rect.size.height = image_rect.size.width * image_size.height /
                                         image_size.width;
                image_rect.pt.x = 0;
                image_rect.pt.y = -(image_rect.size.height - m_thumb_size.height) * 0.5f;
            }
            else
            {
                image_rect.size.height = m_thumb_size.height;
                image_rect.size.width = image_rect.size.height * image_size.width /
                                        image_size.height;
                image_rect.pt.x = -(image_rect.size.width - m_thumb_size.width) * 0.5f;
                image_rect.pt.y = 0;
            }

            const nvg::PaintStyle img_paint{ nvg::image_pattern(
                context, p.x + image_rect.pt.x, p.y + image_rect.pt.y, image_rect.size.width,
                image_rect.size.height, 0, m_images[i].first, m_mouse_index == i ? 1.0f : 0.7f) };

            nvg::begin_path(context);
            nvg::rounded_rect(context, p.x, p.y, m_thumb_size.width, m_thumb_size.height, 5);
            nvg::fill_paint(context, img_paint);
            nvg::fill(context);

            const nvg::PaintStyle shadow_paint{ nvg::box_gradient(
                context, p.x - 1, p.y, m_thumb_size.width + 2.0f, m_thumb_size.height + 2.0f, 5, 3,
                ds::color<f32>{ 0, 0, 0, 128 }, ds::color<f32>{ 0, 0, 0, 0 }) };
            nvg::begin_path(context);
            nvg::rect(context, p.x - 5.0f, p.y - 5.0f, m_thumb_size.width + 10,
                      m_thumb_size.height + 10);
            nvg::rounded_rect(context, p.x, p.y, m_thumb_size.width, m_thumb_size.height, 6);
            nvg::path_winding(context, nvg::Solidity::Hole);
            nvg::fill_paint(context, shadow_paint);
            nvg::fill(context);

            nvg::begin_path(context);
            nvg::rounded_rect(context, p.x + 0.5f, p.y + 0.5f, m_thumb_size.width - 1.0f,
                              m_thumb_size.height - 1.0f, 4.0f - 0.5f);
            nvg::stroke_width(context, 1.0f);
            nvg::stroke_color(context, ds::color<f32>{ 255, 255, 255, 80 });
            nvg::stroke(context);
        }
    }
}
