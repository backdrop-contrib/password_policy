/**
 * @file
 * Password Policy JavaScript functionality for Backdrop CMS.
 */

(function ($, Backdrop) {

  'use strict';

  // Current password status received from the server.
  var pw_status = {
    strength: 0,
    message: '',
    indicatorText: ''
  };

  /**
   * Override the standard password strength check with an AJAX call.
   */
  Backdrop.behaviors.passwordOverride = {
    attach: function (context, settings) {
      settings = settings || Backdrop.settings || {};
      var policySettings = settings.passwordPolicy || {};
      var cleanUrlPrefix = policySettings.cleanUrl ? '' : '?q=';
      var basePath   = settings.basePath   || '/';
      var pathPrefix = settings.pathPrefix || '';

      // Use the attribute we set in hook_form_alter().
      $('input[data-pp-check]', context).once('passwordOverride', function () {
        var $passwordInput = $(this);

        var passwordCheck = function (e, isCallback) {
          // Avoid infinite loop when we re-trigger keyup below.
          if (typeof isCallback !== 'undefined') {
            return;
          }

          // Build POST data.
          var data = { password: encodeURIComponent($passwordInput.val()) };

          // If there's a username field, include it.
          var $usernameInput = $('input.username, input[name="name"]', context);
          var username = $usernameInput.val();
          if (username) {
            data.name = encodeURIComponent(username);
          }

          var url = basePath + cleanUrlPrefix + pathPrefix +
            'password_policy/check' + window.location.search;

          $.post(url, data, function (response) {
            if (response) {
              pw_status = response;
            }
            // Trigger core password.js again so it updates the UI,
            // but pass a dummy param so we don't recurse into this handler.
            $passwordInput.triggerHandler('keyup', [true]);
          }, 'json');
        };

        $passwordInput
          .on('keyup', passwordCheck)
          .on('focusin', passwordCheck);
      });
    }
  };

  /**
   * Backdrop core calls this to evaluate password strength.
   * We just return the last status from the server.
   */
  Backdrop.evaluatePasswordStrength = function (password, translate) {
    return pw_status;
  };

  /**
   * Override the password strength display to show policy compliance.
   */
  Backdrop.behaviors.passwordPolicyDisplay = {
    attach: function (context, settings) {
      // Find all password policy checked inputs.
      $('input[data-pp-check]', context).once('passwordPolicyDisplay', function () {
        var $passwordInput = $(this);
        
        // Update the data-password-strength settings to use "Password compliance:" title.
        if ($passwordInput.attr('data-password-strength')) {
          var strengthSettings = JSON.parse($passwordInput.attr('data-password-strength'));
          if (strengthSettings && strengthSettings.labels) {
            strengthSettings.labels.strengthTitle = Backdrop.t('Password compliance: ');
            $passwordInput.attr('data-password-strength', JSON.stringify(strengthSettings));
          }
        }
        
        // Update the title text if the strength wrapper already exists.
        var updateTitle = function () {
          var $wrapper = $passwordInput.closest('.password-strength-wrapper');
          var $title = $wrapper.find('.password-strength-title');
          if ($title.length > 0) {
            $title.text(Backdrop.t('Password compliance: '));
          }
        };
        
        // Try to update immediately and also after a delay in case the element doesn't exist yet.
        updateTitle();
        setTimeout(updateTitle, 10);
        setTimeout(updateTitle, 100);
        
        // Monitor password input changes and update both indicator and errors.
        $passwordInput.on('keyup.policyDisplay blur.policyDisplay', function () {
          // Use a small delay to ensure the strength text has been updated by core.
          setTimeout(function () {
            var $wrapper = $passwordInput.closest('.password-strength-wrapper');
            var $strengthText = $wrapper.find('.password-strength-text');
            
            // Replace the strength text with our compliance indicator if available.
            if ($strengthText.length > 0 && pw_status && pw_status.indicatorText) {
              $strengthText.text(pw_status.indicatorText);
            }
            
            // Update error messages in the description area.
            if (pw_status && pw_status.message) {
              // Find or create the error message container.
              var $container = $wrapper.closest('.form-item').find('.password-policy-messages');
              if ($container.length === 0) {
                // Create the container if it doesn't exist.
                $container = $('<div class="password-policy-messages"></div>');
                $wrapper.after($container);
              }
              // Update with error messages from the server.
              $container.html(pw_status.message);
            } else {
              // Clear error messages if none.
              var $container = $wrapper.closest('.form-item').find('.password-policy-messages');
              if ($container.length > 0) {
                $container.html('');
              }
            }
          }, 10);
        });
      });
    }
  };

  /**
   * Summary information for constraint settings vertical tabs.
   */
  Backdrop.behaviors.passwordPolicyConstraintSettingsSummary = {
    attach: function (context) {
      $('#edit-alpha-case-fieldset', context).backdropSetSummary(function (ctx) {
        var alpha_case = $('input[name="alpha_case"]', ctx).is(':checked');
        return alpha_case ?
          Backdrop.t('Upper and lower case letters required') :
          Backdrop.t('Not enforced');
      });

      $('#edit-alpha-count-fieldset', context).backdropSetSummary(function (ctx) {
        var alpha_count = $('input[name="alpha_count"]', ctx).val();
        return alpha_count ?
          Backdrop.t('At least @count letters', {'@count': alpha_count}) :
          Backdrop.t('Not enforced');
      });

      $('#edit-delay-fieldset', context).backdropSetSummary(function (ctx) {
        var delay = $('input[name="delay"]', ctx).val();
        var threshold = $('input[name="threshold"]', ctx).val();
        return delay ?
          Backdrop.t('At most @threshold change(s) in @delay', {
            '@threshold': threshold,
            '@delay': delay
          }) :
          Backdrop.t('Not enforced');
      });

      $('#edit-blacklist-fieldset', context).backdropSetSummary(function (ctx) {
        var blacklist = $('textarea[name="blacklist"]', ctx).val();
        if (blacklist) {
          var substr = $('input[name="blacklist_match_substrings"]', ctx).is(':checked');
          return substr ?
            Backdrop.t('Must not contain certain strings') :
            Backdrop.t('Certain strings disallowed');
        }
        return Backdrop.t('Not enforced');
      });

      $('#edit-char-count-fieldset', context).backdropSetSummary(function (ctx) {
        var char_count = $('input[name="char_count"]', ctx).val();
        return char_count ?
          Backdrop.t('At least @count characters', {'@count': char_count}) :
          Backdrop.t('Not enforced');
      });

      $('#edit-consecutive-char-count-fieldset', context).backdropSetSummary(function (ctx) {
        var consecutive_char_count = $('input[name="consecutive_char_count"]', ctx).val();
        return consecutive_char_count ?
          Backdrop.t('Fewer than @count identical consecutive characters', {
            '@count': consecutive_char_count
          }) :
          Backdrop.t('Not enforced');
      });

      $('#edit-backdrop-strength-fieldset', context).backdropSetSummary(function (ctx) {
        var backdrop_strength = $('input[name="backdrop_strength"]', ctx).val();
        return backdrop_strength ?
          Backdrop.t('At least level of @level Backdrop strength', {
            '@level': backdrop_strength
          }) :
          Backdrop.t('Not enforced');
      });

      $('#edit-int-count-fieldset', context).backdropSetSummary(function (ctx) {
        var int_count = $('input[name="int_count"]', ctx).val();
        return int_count ?
          Backdrop.t('At least @count integers', {'@count': int_count}) :
          Backdrop.t('Not enforced');
      });

      $('#edit-past-passwords-fieldset', context).backdropSetSummary(function (ctx) {
        var past_passwords = $('input[name="past_passwords"]', ctx).val();
        return past_passwords ?
          Backdrop.t('Must not match previous @count passwords', {
            '@count': past_passwords
          }) :
          Backdrop.t('Not enforced');
      });

      $('#edit-special-count-fieldset', context).backdropSetSummary(function (ctx) {
        var special_count = $('input[name="special_count"]', ctx).val();
        if (special_count) {
          var chars = $('input[name="special_count_chars"]', ctx).val();
          return Backdrop.t('At least @count special characters', {'@count': special_count}) +
            '<br/>' +
            Backdrop.t('Special characters: @chars', {'@chars': chars});
        }
        return Backdrop.t('Not enforced');
      });

      $('#edit-username-fieldset', context).backdropSetSummary(function (ctx) {
        var username = $('input[name="username"]', ctx).is(':checked');
        return username ?
          Backdrop.t('Must not contain their username') :
          Backdrop.t('Not enforced');
      });
    }
  };

  /**
   * Summary information for condition settings vertical tabs.
   */
  Backdrop.behaviors.passwordPolicyConditionSettingsSummary = {
    attach: function (context) {
      $('#edit-role-fieldset', context).backdropSetSummary(function (ctx) {
        var vals = [];
        $('input[type="checkbox"]:checked', ctx).each(function () {
          vals.push($.trim($(this).next('label').text()));
        });
        if (!vals.length) {
          vals.push(Backdrop.t('Not restricted'));
        }
        return vals.join(', ');
      });

      $('#edit-authmap-fieldset', context).backdropSetSummary(function (ctx) {
        var vals = [];
        $('input[type="checkbox"]:checked', ctx).each(function () {
          vals.push($.trim($(this).next('label').text()));
        });
        if (!vals.length) {
          vals.push(Backdrop.t('Not restricted'));
        }
        return vals.join(', ');
      });

      $('#edit-global-fieldset', context).backdropSetSummary(function (ctx) {
        var global = $('input[name="global"]:checked', ctx);
        if (global.val()) {
          return global.next('label').text();
        }
      });
    }
  };

})(jQuery, Backdrop);
