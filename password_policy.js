(function ($) {
  var B = window.Backdrop || window.Drupal;

  B.behaviors.passwordPolicy = {
    attach: function (context, settings) {
      var base = (settings && settings.basePath) ? settings.basePath : '/';
      var url  = base + 'password_policy/check';

      $('input[type=password][data-pp-check]', context).once('pp-check').each(function () {
        var $input = $(this);

        function check() {
          $.ajax({
            url: url,
            type: 'POST',
            dataType: 'json',
            data: { password: encodeURIComponent($input.val()) },
            success: function (resp) {
              // TODO: update your UI here; for quick proof-of-life:
              var $msg = $('#password-policy-requirements');
              if (resp && resp.message !== undefined) {
                // Put server messages under our requirements list if present,
                // otherwise append after the field.
                if ($msg.length) {
                  $msg.next('.pp-feedback').remove();
                  $('<div class="pp-feedback"></div>').html(resp.message).insertAfter($msg);
                } else {
                  $input.closest('.form-item').find('.pp-feedback').remove();
                  $('<div class="pp-feedback"></div>').html(resp.message).insertAfter($input);
                }
              }
            }
          });
        }

        $input.on('input keyup change', check);
      });
    }
  };
})(jQuery);
