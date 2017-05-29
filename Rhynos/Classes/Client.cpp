#include "Client.h"

#include <cocos2d.h>
#include <ui/UITextField.h>
#include "asio.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/types/vector.hpp"

#include "ClientLevel.h"
#include "GameAction.h"
#include "Level.h"
#include "NetworkingConstants.h"

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

using namespace asio;
using namespace cocos2d;

using cocos2d::ui::TextField;
using std::ostringstream;
using std::string;
using std::to_string;
using std::vector;

bool Client::is_open() const { return conn_->socket().is_open(); }

error_code Client::connect(const string& host) {
  tcp::resolver resolver(conn_->socket().get_io_service());
  error_code ec;
  asio::connect(conn_->socket(),
                resolver.resolve({host, to_string(NetworkingConstants::PORT)}),
                ec);
  return ec;
}

bool Client::available() {
  tcp::socket::bytes_readable command(true);
  conn_->socket().io_control(command);
  return command.get() > 0;
}

bool ClientScene::init() {
  // 1. super init first
  if (!Scene::init()) {
    return false;
  }
  const auto& visibleSize = Director::getInstance()->getVisibleSize();
  const Vec2& origin = Director::getInstance()->getVisibleOrigin();

  // create and initialize a label
  auto host_field =
      TextField::create("Enter the hostname or ip address of the server",
                        "fonts/Marker Felt.ttf", 30);

  // position the label on the center of the screen
  host_field->setPosition(Vec2(
      origin.x + visibleSize.width / 2,
      origin.y + visibleSize.height - host_field->getContentSize().height));

  auto keyListener = EventListenerKeyboard::create();
  keyListener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode,
                                  Event* event) {
    switch (keyCode) {
      case EventKeyboard::KeyCode::KEY_ENTER: {
        auto spritecache = cocos2d::SpriteFrameCache::getInstance();
        spritecache->addSpriteFramesWithFile("images/textures.plist");
        Level* level1 = ClientLevel::createNetworkedWithMap(
            "images/track-2-noai.tmx", host_field->getString());
        level1->loadLayers();
        level1->loadObjects();

        Director::getInstance()->pushScene(level1);
        break;
      }
      default:
        break;
    }
  };

  this->_eventDispatcher->addEventListenerWithSceneGraphPriority(keyListener,
                                                                 this);

  // add the label as a child to this layer
  this->addChild(host_field, 1);

  return true;
}