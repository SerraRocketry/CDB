async function getFiles() {
  const response = await fetch("api/files");
  return await response.json();
}

// Faz uma requisição ao backend de todos os arquivos:
getFiles().then(files => {
  // Pega uma referência a um template HTML referente à uma linha de uma tabela.
  const $template = document.querySelector("#rowTemplate");

  // E para cada arquivo, copia e popula o template com os dados desse arquivo,
  // juntando todas as cópias em $tableRows.
  const $tableRows = files.map(file => {
    const $tableRow = $template.content.cloneNode(true);

    const $rowName = $tableRow.querySelector(".rowTemplateName");
    $rowName.textContent = file.name;

    const $rowSize = $tableRow.querySelector(".rowTemplateSize");
    $rowSize.textContent = humanFileSize(file.size);

    const $openLink = $tableRow.querySelector(".rowTemplateOpen");
    const fileURL = new URL("/api/file", document.location);
    fileURL.searchParams.set("filename", file.name);
    $openLink.href = fileURL.href;

    const $downloadLink = $tableRow.querySelector(".rowTemplateDownload");
    fileURL.searchParams.set("download", 1); // Reusando URL por praticidade.
    $downloadLink.href = fileURL.href;

    return $tableRow;
  });

  // Por fim, popule a tabela com essas linhas.
  document.querySelector("#tableBody").replaceChildren(...$tableRows);
})

// Roubei do StackOverflow lol.
function humanFileSize(size) {
  var i = size == 0 ? 0 : Math.floor(Math.log(size) / Math.log(1024));
  return +((size / Math.pow(1024, i)).toFixed(2)) * 1 +
    ' ' + ['B', 'kB', 'MB', 'GB', 'TB'][i];
}
