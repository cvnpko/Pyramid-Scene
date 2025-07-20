//
// Created by cvnpko on 7/20/25.
//

#ifndef BLOOMOBSERVER_HPP
#define BLOOMOBSERVER_HPP
#include <engine/platform/PlatformEventObserver.hpp>

namespace engine::platform {
class BloomObserver final : public PlatformEventObserver {
public:
    void on_mouse_move(MousePosition position);

    void on_scroll(MousePosition position);

    void on_key(Key key);

    void on_window_resize(int width, int height);

};
}
#endif //BLOOMOBSERVER_HPP
