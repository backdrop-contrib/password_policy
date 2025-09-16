# Password Policy

The Password Policy module allows administrators to define and enforce password
policies for user passwords. A password policy comprises (1) constraints on
password composition, (2) conditions that determine to which users it should
apply, and (3) items that provide other settings (e.g., expiration).

The Password Policy module includes an example policy that is enabled by
default and applies to all users. The module also includes constraints,
conditions, and items that an administrator can use to define their own
policies.

## Installation

- Install this module using the [official Backdrop CMS instructions](https://backdropcms.org/guide/modules).

## Configuration

Configure policies at Administration » Configuration » User accounts » Password
policies.

An example policy is provided and enabled by default. To customize the default
policy there are two options:

    1. Edit the example policy using the "Edit" link in the "Operations"
       column menu.

    2. Disable the example policy using the "Disable" link in the "Operations"
       column menu. Then, add a new policy.

You can add a policy using the "Add" link at the top of the page.

You can duplicate a policy using the "Clone" link in the "Operations" column
menu. This can be useful if you would like to have similar, but not identical,
policies that apply to different roles.

## Issues

Bugs and Feature requests should be reported in the
[Issue Queue](https://github.com/backdrop-contrib/password_policy/issues)

## Current Maintainers

- [Justin Keiser](https://github.com/keiserjb)

## Credits

- Ported to Backdrop by [Justin Keiser](https://github.com/keiserjb)
- Maintained for Drupal by
  - [aihorvetpv](https://www.drupal.org/u/aohrvetpv)
  - [deekayen]('https://www.drupal.org/u/deekayen')
  - [miglius]('https://www.drupal.org/u/miglius')
  - [nerdstein]('https://www.drupal.org/u/nerdstein')
  - [shrop]('https://www.drupal.org/u/shrop')
  - [vishalkhode]('https://www.drupal.org/u/vishalkhode')
  - [kristen pol]('https://www.drupal.org/u/kristen-pol')
  - [paulocs]('https://www.drupal.org/u/paulocs')
  - [dayre]('https://www.drupal.org/u/dayre')

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.
