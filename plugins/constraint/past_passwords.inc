<?php

/**
 * @file
 * Password history constraint for Password Policy module (Backdrop CMS).
 */

$plugin = array(
  'admin form callback' => 'password_policy_past_passwords_count_admin_form',
  'constraint callback' => 'password_policy_past_passwords_count_constraint',
  'message' => t('Password cannot match @past_passwords past passwords.'),
  'prime value' => 'past_passwords',
  'config' => array(
    'past_passwords' => NULL,
  ),
);

/**
 * Admin form callback for password history constraint.
 */
function password_policy_past_passwords_count_admin_form($form, &$form_state, $constraint) {
  $sub_form['past_passwords_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Past passwords'),
  );
  $sub_form['past_passwords_fieldset']['past_passwords'] = array(
    '#type' => 'textfield',
    '#title' => t('Check against previous passwords'),
    '#default_value' => isset($constraint->config['past_passwords']) ? $constraint->config['past_passwords'] : 0,
    '#description' => t('Password cannot match this many previous passwords.'),
  );

  return $sub_form;
}

/**
 * Constraint callback for password history constraint.
 *
 * @param string $password
 *   The plain-text password the user is trying to set.
 * @param object $account
 *   The user account object.
 * @param object $constraint
 *   The constraint configuration object.
 *
 * @return bool
 *   TRUE if the password does not match any of the last N passwords.
 *   FALSE if it matches one of them.
 */
function password_policy_past_passwords_count_constraint($password, $account, $constraint) {
  global $user;

  // Fallback to the global user if no account was passed.
  if (!$account) {
    $account = $user;
  }

  // Ensure Backdrop's password API is available.
  require_once BACKDROP_ROOT . '/' . settings_get('password_inc', 'core/includes/password.inc');

  $count = (int) $constraint->config['past_passwords'];

  // Nothing to enforce if no count configured or no real user.
  if ($count <= 0 || empty($account->uid)) {
    return TRUE;
  }

  // Ensure password_history is loaded on the account.
  if (!isset($account->password_history)) {
    password_policy_user_load(array($account->uid => $account));
  }

  if (empty($account->password_history) || !is_array($account->password_history)) {
    return TRUE;
  }

  // Check only the last $count history entries.
  for ($i = 0; $i < $count; $i++) {
    if (!isset($account->password_history[$i])) {
      // No more history at this depth.
      break;
    }

    $entry = $account->password_history[$i];

    // Normalize to an object with a 'pass' property for user_check_password().
    if (is_object($entry) && isset($entry->pass)) {
      $history_account = (object) array(
        'uid'  => $account->uid,
        'pass' => $entry->pass,
      );
    }
    elseif (is_string($entry)) {
      $history_account = (object) array(
        'uid'  => $account->uid,
        'pass' => $entry,
      );
    }
    else {
      // Unexpected format; skip this entry.
      continue;
    }

    // Compare the new plain-text password to the stored hash.
    if (user_check_password($password, $history_account)) {
      // Match found in history → constraint fails.
      return FALSE;
    }
  }

  // No matches found in the last N history entries → constraint passes.
  return TRUE;
}
