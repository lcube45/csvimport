<?php

use \Drupal\node\Entity\Node;

/**
 * Handle batch completion.
 *
 *   Creates a new CSV file containing all failed rows if any.
 */
function csvimport_import_finished($success, $results, $operations) {

  if (!empty($results['failed_rows'])) {

    $dir = 'public://csvimport';
    if (file_prepare_directory($dir, FILE_CREATE_DIRECTORY)) {

      $csv_filename = 'failed_rows-' . basename($results['uploaded_filename']); // we validated extension on upload
      $csv_filepath = $dir . '/' . $csv_filename;

      $targs = [
        ':csv_url'      => file_create_url($csv_filepath),
        '@csv_filename' => $csv_filename,
        '@csv_filepath' => $csv_filepath,
      ];

      if ($handle = fopen($csv_filepath, 'w+')) {

        foreach ($results['failed_rows'] as $failed_row) {
          fputcsv($handle, $failed_row);
        }

        fclose($handle);
        drupal_set_message(t('Some rows failed to import. You may download a CSV of these rows: <a href=":csv_url">@csv_filename</a>', $targs), 'error');
      }
      else {
        drupal_set_message(t('Some rows failed to import, but unable to write error CSV to @csv_filepath', $targs), 'error');
      }
    }
    else {
      drupal_set_message(t('Some rows failed to import, but unable to create directory for error CSV at @csv_directory', $targs), 'error');
    }
  }

  return t('The CSV import has completed.');
}

/**
 * Remember the uploaded CSV filename.
 *
 * @TODO Is there a better way to pass a value from inception of the batch to
 * the finished function?
 */
function _csvimport_remember_filename($filename, &$context) {

  $context['results']['uploaded_filename'] = $filename;
}

/**
 * Process a single line.
 */
function _csvimport_import_line($line, &$context) {

  $context['results']['rows_imported']++;
  $line = $cleaned_line = array_map('base64_decode', $line);

  /**
   * Simply show the import row count.
   */
  $context['message'] = t('Importing row !c', ['!c' => $context['results']['rows_imported']]);

  /**
   * Alternatively, our example CSV happens to have the title in the
   * third column, so we can uncomment this line to display "Importing
   * Blahblah" as each row is parsed.
   *
   * You can comment out the line above if you uncomment this one.
   */
  $context['message'] = t('Importing %title', ['%title' => $line[2]]);

  /**
   * In order to slow importing and debug better, we can uncomment
   * this line to make each import slightly slower.
   */
  usleep(2500);

  /**
   * Convert the line of the CSV file into a new node.
   *
   */
  // if ($context['results']['rows_imported'] > 1) { // Skip header line.
  //   $node = Node::create([
  //     'type'  => 'article',
  //     'title' => $line[2],
  //     'body'  => $line[0],
  //   ]);
  //
  //   $node->save();
  // }

  /**
   * If the first two columns in the row are "ROW", "FAILS" then we
   * will add that row to the CSV we'll return to the importing person
   * after the import completes.
   */
  if ($line[1] == 'ROW' && $line[2] == 'FAILS') {
    $context['results']['failed_rows'][] = $line;
  }

}
