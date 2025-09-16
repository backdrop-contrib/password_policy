<?php

/**
 * @file
 * Role-based condition for Password Policy module.
 */

$plugin = array(
  'admin form callback' => 'password_policy_role_admin_form',
  'condition callback' => 'password_policy_role_condition',
  'prime value' => 'role_char_count',
  'config' => array(
    'roles' => array(),
  ),
);

/**
 * Admin form callback for role condition.
 */
function password_policy_role_admin_form($form, &$form_state, $condition) {
  $sub_form['role_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Roles'),
  );
  // Only enabled roles should be checked in the form
  $default_roles = array();
  if (!empty($condition->config['roles'])) {
    foreach ($condition->config['roles'] as $rid => $value) {
      if (!empty($value) && $value !== '0') {
        $default_roles[$rid] = $rid;
      }
    }
  }
  $sub_form['role_fieldset']['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enforce policy for specific roles'),
    '#default_value' => $default_roles,
    '#description' => t('Enforce this policy only for the selected role(s). If you select no roles, the policy will be enforced for all users.'),
    '#options' => user_roles(FALSE),
  );
  return $sub_form;
}

/**
 * Condition callback for role condition.
 */
function password_policy_role_condition($account, $condition) {
  $roles = array_filter($condition->config['roles']);

  // Always enforce this policy if no roles were selected.
  if (count($roles) == 0) {
    return TRUE;
  }

  // Normalize account roles to associative array keyed by machine name
  $normalized_roles = array();
  if (!empty($account->roles)) {
    foreach ($account->roles as $role) {
      $normalized_roles[$role] = $role;
    }
  }

  // Test if policy roles intersect with account roles.
  return count(array_intersect_key($normalized_roles, $roles));
}
