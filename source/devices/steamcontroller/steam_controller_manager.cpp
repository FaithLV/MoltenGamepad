#ifdef BUILD_STEAM_CONTROLLER_DRIVER
#include "steam_controller.h"
#include "../../event_translators/translators.h"

steam_controller_manager::steam_controller_manager(moltengamepad* mg) : device_manager(mg,"steamcontroller"), sc_context(std::bind(&steam_controller_manager::on_controller_gained, this, std::placeholders::_1), std::bind(&steam_controller_manager::on_controller_lost, this, std::placeholders::_1)) {

  init_profile();
  //need to do a thread for hot plugs, since these don't go through udev.
  keep_scanning = true;

  sc_context_thread = new std::thread([this] () {
    while (keep_scanning) {
      sc_context.handle_events(1000);
    }
  });

}

void steam_controller_manager::on_controller_gained(scraw::controller sc) {
  scraw::controller* ref = new scraw::controller(sc);
  steam_controller* steamcont = new steam_controller(ref, mg->slots, this);
  sc_devs[sc] = steamcont;
  mg->add_device(steamcont, this, "sc");
}
void steam_controller_manager::on_controller_lost(scraw::controller sc) {
  auto dev = sc_devs.find(sc);
  if (dev != sc_devs.end()) {
    mg->remove_device(dev->second);
    sc_devs.erase(dev);
  }
}


void steam_controller_manager::init_profile() {
  //Init some event translators

  const event_decl* ev = &steamcont_events[0];
  for (int i = 0; ev->name && *ev->name; ev = &steamcont_events[++i]) {
    register_event(*ev);
  }
 
  //Init some aliases to act like a standardized game pad
  mapprofile->set_alias("primary","a");
  mapprofile->set_alias("secondary","b");
  mapprofile->set_alias("third","x");
  mapprofile->set_alias("fourth","y");
  mapprofile->set_alias("leftright","left_pad_x");
  mapprofile->set_alias("updown","left_pad_y");
  mapprofile->set_alias("start","forward");
  mapprofile->set_alias("select","back");
  mapprofile->set_alias("thumbl","stick_click");
  mapprofile->set_alias("thumbr","right_pad_click");
  mapprofile->set_alias("left_x","stick_x");
  mapprofile->set_alias("left_y","stick_y");
  mapprofile->set_alias("right_x","right_pad_x");
  mapprofile->set_alias("right_y","right_pad_y");
  
  mg->gamepad->copy_into(mapprofile, true, false);

};
#endif
