<?php

/**
 * @file
 * Backdrop-native PasswordPolicy manager (no CTools).
 */

class PasswordPolicy {
  public $name = '';
  public $config = [];
  public $items = [];
  public $cache = [];

  /**
   * Constructor.
   *
   * @param object|null $policy
   *   Backdrop-style policy object (e.g., from config), or NULL for a blank.
   */
  public function __construct($policy = NULL) {
    if ($policy === NULL) {
      // Create a blank policy object (Backdrop style).
      $policy = (object) [
        'name'   => '',
        'config' => [],
        // Optional flags your code may reference:
        // 'disabled' => FALSE,
      ];
    }

    $this->name = isset($policy->name) ? $policy->name : '';

    // Normalize config: accept array (Backdrop) or serialized string (legacy).
    $this->config = [];
    if (isset($policy->config)) {
      if (is_array($policy->config)) {
        $this->config = $policy->config;
      }
      elseif (is_string($policy->config)) {
        $maybe = @unserialize($policy->config);
        $this->config = is_array($maybe) ? $maybe : [];
      }
    }

    // Load all constraint/condition/item plugins via Plugin Manager.
    $this->loadPlugins();
  }

  /**
   * Admin form builder.
   */
  public function adminForm(&$form, &$form_state) {
    // Conditions.
    $form['condition_title'] = [
      '#type' => 'item',
      '#title' => t('Conditions'),
    ];
    $form['condition'] = [
      '#type' => 'vertical_tabs',
      '#attached' => [
        'js' => [backdrop_get_path('module', 'password_policy') . '/password_policy.js'],
      ],
    ];

    // Constraints.
    $form['constraint_title'] = [
      '#type' => 'item',
      '#title' => t('Constraints'),
    ];
    $form['constraint'] = [
      '#type' => 'vertical_tabs',
      '#attached' => [
        'js' => [backdrop_get_path('module', 'password_policy') . '/password_policy.js'],
      ],
    ];

    // Other items.
    $form['item_title'] = [
      '#type' => 'item',
      '#title' => t('Other settings'),
    ];
    $form['item'] = [
      '#type' => 'container',
    ];

    foreach ($this->items as $item) {
      $parent = $item->isType('condition') ? 'condition' : ($item->isType('constraint') ? 'constraint' : 'item');
      $form[$parent] = $form[$parent] + $item->adminForm($form, $form_state);
    }

    return $form;
  }

  /**
   * Admin form submit: collect plugin configs.
   */
  public function adminFormSubmit($form, &$form_state) {
    foreach ($this->items as $item) {
      $item->adminFormSubmit($form, $form_state);
    }
    // Remove role value conversion here! Plugin submit handler sets correct format.
    if (isset($form_state['item'])) {
      $form_state['item']->config = $this->config;
    }
  }

  /**
   * Get messages from active constraints.
   *
   * @return string[]
   */
  public function messages() {
    $items = [];
    foreach ($this->activeConstraints() as $constraint) {
      $items = array_merge($items, $constraint->messages());
    }
    return $items;
  }

  /**
   * Check password against all active constraints.
   *
   * @return string[] Failure messages.
   */
  public function check($password, $account) {
    $errors = [];
    foreach ($this->activeConstraints() as $constraint) {
      if (!$constraint->check($password, $account)) {
        $errors = array_merge($errors, $constraint->failMessages($password, $account));
      }
    }
    return $errors;
  }

  /**
   * @return PasswordPolicyConstraint[] Active constraints.
   */
  public function activeConstraints() {
    $constraints = [];
    foreach ($this->getItems('constraint') as $constraint) {
      if ($constraint->isActive()) {
        $constraints[] = $constraint;
      }
    }
    return $constraints;
  }

  /**
   * TRUE if all active conditions match the account.
   */
  public function match($account) {
    foreach ($this->getItems('condition') as $condition) {
      // Any condition with non-default configuration is "active".
      if ($condition->info['config'] != $condition->config) {
        if (!$condition->match($account)) {
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  /**
   * Run cron hooks for active cron-type items.
   */
  public function cron() {
    foreach ($this->getItems('cron') as $item) {
      if ($item->isActive()) {
        $item->cron();
      }
    }
  }

  /**
   * Run init hooks for init-type items.
   */
  public function init($account) {
    foreach ($this->getItems('init') as $item) {
      $item->init($account);
    }
  }

  /**
   * Discover plugins via Plugin Manager.
   */
  private function loadPlugins() {
    foreach (['constraint','condition','item'] as $type) {
      $dir = backdrop_get_path('module', 'password_policy') . "/plugins/$type";
      if (!is_dir($dir)) {
        watchdog('password_policy_debug', "SKIP type '$type' — directory missing: @dir", ['@dir' => $dir], WATCHDOG_WARNING);
        continue;
      }

      try {
        $defs = plugin_manager_get_plugins('password_policy', $type);
        //watchdog('password_policy_debug', "FOUND for '$type' (@count)",
        // ['@count' => count($defs)]);
      }
      catch (Exception $e) {
        watchdog('password_policy_debug', "plugin_manager_get_plugins error for '$type': @msg", ['@msg' => $e->getMessage()], WATCHDOG_ERROR);
        continue;
      }

      foreach ($defs as $info) {
        $this->loadPlugin($info);
      }
    }
  }

  /**
   * Instantiate a plugin.
   *
   * @param array $info
   */
  private function loadPlugin($info) {
    if (!isset($this->config[$info['name']])) {
      $this->config[$info['name']] = array();
    }
    // Default weight from our map unless the plugin already provided one.
    if (!isset($info['weight'])) {
      $map = _password_policy_weight_map();
      $info['weight'] = isset($map[$info['name']]) ? (int) $map[$info['name']] : 0;
    }
    $this->items[$info['name']] = PasswordPolicyConstraint::factory($info, $this);
  }


  /**
   * Filter items by type (constraint/condition/item/cron/init).
   *
   * @return PasswordPolicyItem[]
   */
  private function getItems($type) {
    $this->cache[$type] = isset($this->cache[$type]) ? $this->cache[$type] : [];
    if (empty($this->cache[$type])) {
      foreach ($this->items as $item) {
        if ($item->isType($type)) {
          $this->cache[$type][] = $item;
        }
      }
    }
    return $this->cache[$type];
  }

  /**
   * Load one policy from config by name.
   *
   * @return PasswordPolicy|null
   */
  public static function policy($name) {
    $info = config_get('password_policy.' . $name);
    if ($info) {
      return new PasswordPolicy((object) $info);
    }
    return NULL;
  }

  /**
   * Load all policies from config.
   *
   * @return PasswordPolicy[]
   */
  public static function allPolicies() {
    $policies = [];
    $config_names = config_get_names_with_prefix('password_policy.');
    foreach ($config_names as $config_name) {
      $info = config($config_name)->get();
      $name = str_replace('password_policy.', '', $config_name);
      $policies[$name] = new PasswordPolicy((object) $info);
    }
    return $policies;
  }

  /**
   * Load all enabled policies from config.
   *
   * @return PasswordPolicy[]
   */
  public static function enabledPolicies() {
    $policies = [];
    $config_names = config_get_names_with_prefix('password_policy.');
    foreach ($config_names as $config_name) {
      $info = config($config_name)->get();
      if (empty($info['disabled'])) {
        $name = str_replace('password_policy.', '', $config_name);
        $policies[$name] = new PasswordPolicy((object) $info);
      }
    }
    return $policies;
  }

  /**
   * Load all enabled policies that match an account.
   *
   * @return PasswordPolicy[]
   */
  public static function matchedPolicies($account) {
    $policies = [];
    foreach (PasswordPolicy::enabledPolicies() as $policy) {
      if ($policy->match($account)) {
        $policies[] = $policy;
      }
    }
    return $policies;
  }

}
