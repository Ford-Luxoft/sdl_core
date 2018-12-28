/*
 * Copyright (c) 2018, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>
#include <string>

#include "gtest/gtest.h"

#include "application_manager/commands/commands_test.h"
#include "application_manager/commands/command_request_test.h"
#include "application_manager/mock_application.h"

#include "mobile/show_app_menu_request.h"

#include "interfaces/MOBILE_API.h"

namespace test {
namespace components {
namespace commands_test {
namespace mobile_commands_test {
namespace show_app_menu_request {

namespace am = ::application_manager;
using sdl_rpc_plugin::commands::ShowAppMenuRequest;

namespace {
const uint32_t kConnectionKey = 2u;
const uint32_t kAppId = 1u;
}  // namespace

class ShowAppMenuRequestTest
    : public CommandRequestTest<CommandsTestMocks::kIsNice> {
 public:
  ShowAppMenuRequestTest() : mock_app(CreateMockApp()) {
    EXPECT_CALL(app_mngr_, application(kConnectionKey))
        .WillRepeatedly(Return(mock_app));
  }

  MockAppPtr mock_app;

  MessageSharedPtr CreateMsgParams() {
    MessageSharedPtr msg = CreateMessage();
    (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;
    (*msg)[am::strings::msg_params][am::strings::app_id] = kAppId;
    return msg;
  }
};

TEST_F(ShowAppMenuRequestTest, Testig_Succes_Request_From_Mobile_To_Hmi) {
  const uint32_t menu_id = 10u;
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;
  (*msg)[am::strings::msg_params][am::strings::menu_id] = menu_id;

  ON_CALL(app_mngr_, application(kConnectionKey))
      .WillByDefault(Return(mock_app));
  ON_CALL(*mock_app, hmi_level())
      .WillByDefault(Return(mobile_apis::HMILevel::HMI_FULL));
  ON_CALL(*mock_app, app_id()).WillByDefault(Return(kConnectionKey));
  ON_CALL(*mock_app, system_context())
      .WillByDefault(Return(mobile_apis::SystemContext::SYSCTXT_MAIN));

  smart_objects::SmartObject sub_menu_;
  ON_CALL(*mock_app, FindSubMenu(menu_id)).WillByDefault(Return(&sub_menu_));

  MessageSharedPtr ui_command_result;

  EXPECT_CALL(mock_rpc_service_, ManageHMICommand(_))
      .WillOnce(DoAll(SaveArg<0>(&ui_command_result), Return(true)));

  std::shared_ptr<ShowAppMenuRequest> request_ptr =
      CreateCommand<ShowAppMenuRequest>(msg);

  request_ptr->Run();

  EXPECT_EQ((*ui_command_result)[am::strings::msg_params][am::strings::menu_id]
                .asInt(),
            menu_id);
}

TEST_F(ShowAppMenuRequestTest, Testig_Failed_Application_Not_Registered) {
  const uint32_t menu_id = 10u;
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;
  (*msg)[am::strings::msg_params][am::strings::menu_id] = menu_id;

  std::shared_ptr<am::Application> null_application_sptr;
  EXPECT_CALL(app_mngr_, application(kConnectionKey))
      .WillOnce(Return(null_application_sptr));

  MessageSharedPtr ui_command_result;

  EXPECT_CALL(
      mock_rpc_service_,
      ManageMobileCommand(_, am::commands::Command::CommandSource::SOURCE_SDL))
      .WillOnce(DoAll(SaveArg<0>(&ui_command_result), Return(true)));

  std::shared_ptr<ShowAppMenuRequest> request_ptr =
      CreateCommand<ShowAppMenuRequest>(msg);

  request_ptr->Run();

  EXPECT_EQ((*ui_command_result)[am::strings::msg_params][am::strings::success]
                .asBool(),
            false);
  EXPECT_EQ(
      (*ui_command_result)[am::strings::msg_params][am::strings::result_code]
          .asInt(),
      static_cast<int32_t>(mobile_apis::Result::APPLICATION_NOT_REGISTERED));
}

TEST_F(ShowAppMenuRequestTest, Testig_Fail_Wrong_System_Context) {
  const uint32_t menu_id = 10u;
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;
  (*msg)[am::strings::msg_params][am::strings::menu_id] = menu_id;

  ON_CALL(app_mngr_, application(kConnectionKey))
      .WillByDefault(Return(mock_app));
  ON_CALL(*mock_app, hmi_level())
      .WillByDefault(Return(mobile_apis::HMILevel::HMI_FULL));
  ON_CALL(*mock_app, system_context())
      .WillByDefault(Return(mobile_apis::SystemContext::INVALID_ENUM));

  MessageSharedPtr ui_command_result;

  EXPECT_CALL(
      mock_rpc_service_,
      ManageMobileCommand(_, am::commands::Command::CommandSource::SOURCE_SDL))
      .WillOnce(DoAll(SaveArg<0>(&ui_command_result), Return(true)));

  std::shared_ptr<ShowAppMenuRequest> request_ptr =
      CreateCommand<ShowAppMenuRequest>(msg);

  request_ptr->Run();

  EXPECT_EQ((*ui_command_result)[am::strings::msg_params][am::strings::success]
                .asBool(),
            false);
  EXPECT_EQ(
      (*ui_command_result)[am::strings::msg_params][am::strings::result_code]
          .asInt(),
      static_cast<int32_t>(mobile_apis::Result::REJECTED));
}

TEST_F(ShowAppMenuRequestTest, Testig_Fail_Wrong_HMI_Level) {
  const uint32_t menu_id = 10u;
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;
  (*msg)[am::strings::msg_params][am::strings::menu_id] = menu_id;

  ON_CALL(app_mngr_, application(kConnectionKey))
      .WillByDefault(Return(mock_app));
  ON_CALL(*mock_app, hmi_level())
      .WillByDefault(Return(mobile_apis::HMILevel::INVALID_ENUM));

  MessageSharedPtr ui_command_result;

  EXPECT_CALL(
      mock_rpc_service_,
      ManageMobileCommand(_, am::commands::Command::CommandSource::SOURCE_SDL))
      .WillOnce(DoAll(SaveArg<0>(&ui_command_result), Return(true)));

  std::shared_ptr<ShowAppMenuRequest> request_ptr =
      CreateCommand<ShowAppMenuRequest>(msg);

  request_ptr->Run();

  EXPECT_EQ((*ui_command_result)[am::strings::msg_params][am::strings::success]
                .asBool(),
            false);
  EXPECT_EQ(
      (*ui_command_result)[am::strings::msg_params][am::strings::result_code]
          .asInt(),
      static_cast<int32_t>(mobile_apis::Result::REJECTED));
}

TEST_F(ShowAppMenuRequestTest, Testig_Sending_From_Mobile_Invalid_MenuId) {
  const uint32_t menu_id = 10u;
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;
  (*msg)[am::strings::msg_params][am::strings::menu_id] = menu_id;

  ON_CALL(app_mngr_, application(kConnectionKey))
      .WillByDefault(Return(mock_app));
  ON_CALL(*mock_app, hmi_level())
      .WillByDefault(Return(mobile_apis::HMILevel::HMI_FULL));
  ON_CALL(*mock_app, app_id()).WillByDefault(Return(kConnectionKey));
  ON_CALL(*mock_app, system_context())
      .WillByDefault(Return(mobile_apis::SystemContext::SYSCTXT_MAIN));

  smart_objects::SmartObject* null_sub_menu_sptr = NULL;
  ON_CALL(*mock_app, FindSubMenu(menu_id))
      .WillByDefault(Return(null_sub_menu_sptr));

  MessageSharedPtr ui_command_result;

  EXPECT_CALL(
      mock_rpc_service_,
      ManageMobileCommand(_, am::commands::Command::CommandSource::SOURCE_SDL))
      .WillOnce(DoAll(SaveArg<0>(&ui_command_result), Return(true)));

  std::shared_ptr<ShowAppMenuRequest> request_ptr =
      CreateCommand<ShowAppMenuRequest>(msg);

  request_ptr->Run();

  EXPECT_EQ((*ui_command_result)[am::strings::msg_params][am::strings::success]
                .asBool(),
            false);
  EXPECT_EQ(
      (*ui_command_result)[am::strings::msg_params][am::strings::result_code]
          .asInt(),
      static_cast<int32_t>(mobile_apis::Result::INVALID_ID));
}

TEST_F(ShowAppMenuRequestTest, OnEvent_UI_SUCCESS) {
  MessageSharedPtr msg = CreateMessage(smart_objects::SmartType_Map);
  (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;

  std::shared_ptr<ShowAppMenuRequest> command =
      CreateCommand<ShowAppMenuRequest>(msg);

  ON_CALL(app_mngr_, application(kConnectionKey))
      .WillByDefault(Return(mock_app));
  ON_CALL(*mock_app, app_id()).WillByDefault(Return(kConnectionKey));

  MessageSharedPtr ev_msg = CreateMessage(smart_objects::SmartType_Map);
  (*ev_msg)[am::strings::params][am::hmi_response::code] =
      hmi_apis::Common_Result::SUCCESS;
  (*ev_msg)[am::strings::msg_params][am::strings::info] = "info";

  Event event(hmi_apis::FunctionID::UI_ShowAppMenu);
  event.set_smart_object(*ev_msg);

  MessageSharedPtr ui_command_result;
  EXPECT_CALL(
      mock_rpc_service_,
      ManageMobileCommand(_, am::commands::Command::CommandSource::SOURCE_SDL))
      .WillOnce(DoAll(SaveArg<0>(&ui_command_result), Return(true)));
  command->Init();
  command->on_event(event);

  EXPECT_EQ((*ui_command_result)[am::strings::msg_params][am::strings::success]
                .asBool(),
            true);
  EXPECT_EQ(
      (*ui_command_result)[am::strings::msg_params][am::strings::result_code]
          .asInt(),
      static_cast<int32_t>(hmi_apis::Common_Result::SUCCESS));
  EXPECT_EQ((*ui_command_result)[am::strings::msg_params][am::strings::info]
                .asString(),
            "info");
}

}  // namespace show_app_menu_request
}  // namespace mobile_commands_test
}  // namespace commands_test
}  // namespace components
}  // namespace test
