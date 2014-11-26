// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_ENROLLMENT_ENROLLMENT_SCREEN_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_ENROLLMENT_ENROLLMENT_SCREEN_H_

#include <string>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/gtest_prod_util.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/chromeos/login/enrollment/enrollment_mode.h"
#include "chrome/browser/chromeos/login/enrollment/enrollment_screen_actor.h"
#include "chrome/browser/chromeos/login/enrollment/enterprise_enrollment_helper.h"
#include "chrome/browser/chromeos/login/screens/base_screen.h"
#include "components/pairing/host_pairing_controller.h"
#include "components/policy/core/common/cloud/cloud_policy_constants.h"
#include "components/policy/core/common/cloud/enterprise_metrics.h"

namespace base {
class ElapsedTimer;
}

namespace pairing_chromeos {
class ControllerPairingController;
}

namespace chromeos {

class BaseScreenDelegate;
class ScreenManager;

// The screen implementation that links the enterprise enrollment UI into the
// OOBE wizard.
class EnrollmentScreen
    : public BaseScreen,
      public pairing_chromeos::HostPairingController::Observer,
      public EnterpriseEnrollmentHelper::EnrollmentStatusConsumer,
      public EnrollmentScreenActor::Controller {
 public:
  typedef pairing_chromeos::HostPairingController::Stage Stage;

  EnrollmentScreen(BaseScreenDelegate* base_screen_delegate,
                   EnrollmentScreenActor* actor);
  virtual ~EnrollmentScreen();

  static EnrollmentScreen* Get(ScreenManager* manager);

  // Setup how this screen will handle enrollment.
  //   |shark_controller| is an interface that is used to communicate with a
  //     remora device for remote enrollment.
  //   |remora_controller| is an interface that is used to communicate with a
  //     shark device for remote enrollment.
  void SetParameters(
      EnrollmentMode enrollment_mode,
      const std::string& management_domain,
      const std::string& enrollment_user,
      pairing_chromeos::ControllerPairingController* shark_controller,
      pairing_chromeos::HostPairingController* remora_controller);

  // BaseScreen implementation:
  virtual void PrepareToShow() override;
  virtual void Show() override;
  virtual void Hide() override;
  virtual std::string GetName() const override;

  // pairing_chromeos::HostPairingController::Observer:
  virtual void PairingStageChanged(Stage new_stage) override;
  virtual void ConfigureHost(bool accepted_eula,
                             const std::string& lang,
                             const std::string& timezone,
                             bool send_reports,
                             const std::string& keyboard_layout) override;
  virtual void EnrollHost(const std::string& auth_token) override;

  // EnrollmentScreenActor::Controller implementation:
  virtual void OnLoginDone(const std::string& user) override;
  virtual void OnRetry() override;
  virtual void OnCancel() override;
  virtual void OnConfirmationClosed() override;

  // EnterpriseEnrollmentHelper::EnrollmentStatusConsumer implementation:
  virtual void OnAuthError(const GoogleServiceAuthError& error) override;
  virtual void OnEnrollmentError(policy::EnrollmentStatus status) override;
  virtual void OnOtherError(
      EnterpriseEnrollmentHelper::OtherError error) override;
  virtual void OnDeviceEnrolled(const std::string& additional_token) override;

  // Used for testing.
  EnrollmentScreenActor* GetActor() {
    return actor_;
  }

 private:
  FRIEND_TEST_ALL_PREFIXES(EnrollmentScreenTest, TestSuccess);

  // Creates an enrollment helper.
  void CreateEnrollmentHelper();

  // Clears auth in |enrollment_helper_|. Deletes |enrollment_helper_| and runs
  // |callback| on completion. See the comment for
  // EnterpriseEnrollmentHelper::ClearAuth for details.
  void ClearAuth(const base::Closure& callback);

  // Used as a callback for EnterpriseEnrollmentHelper::ClearAuth.
  virtual void OnAuthCleared(const base::Closure& callback);

  // Sends an enrollment access token to a remote device.
  void SendEnrollmentAuthToken(const std::string& token);

  // Shows successful enrollment status after all enrollment related file
  // operations are completed.
  void ShowEnrollmentStatusOnSuccess();

  // Logs an UMA event in one of the "Enrollment.*" histograms, depending on
  // |enrollment_mode_|.
  void UMA(policy::MetricEnrollment sample);

  // Shows the signin screen. Used as a callback to run after auth reset.
  void ShowSigninScreen();

  // Convenience helper to check for auto enrollment mode.
  bool is_auto_enrollment() const {
    return enrollment_mode_ == ENROLLMENT_MODE_AUTO;
  }

  void OnAnyEnrollmentError();

  pairing_chromeos::ControllerPairingController* shark_controller_;
  pairing_chromeos::HostPairingController* remora_controller_;
  EnrollmentScreenActor* actor_;
  EnrollmentMode enrollment_mode_;
  bool enrollment_failed_once_;
  std::string user_;
  scoped_ptr<base::ElapsedTimer> elapsed_timer_;
  scoped_ptr<EnterpriseEnrollmentHelper> enrollment_helper_;
  base::WeakPtrFactory<EnrollmentScreen> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(EnrollmentScreen);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_ENROLLMENT_ENROLLMENT_SCREEN_H_
