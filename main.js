marked.use({ breaks: true })
async function fetchReadme() {
  try {
    const response = await fetch('https://raw.githubusercontent.com/Lunar-Eclipse255/maelstrom/main/README.md');
    if (!response.ok) throw new Error('README not found');
    const text = await response.text();
    document.getElementById('readme-content').innerHTML = marked.parse(text);
  } catch (error) {
    console.error('Error fetching README:', error);
  }
}

async function fetchChangelog() {
  try {
    const response = await fetch('https://raw.githubusercontent.com/Lunar-Eclipse255/maelstrom/main/changelog.md');
    if (!response.ok) throw new Error('Changelog not found');
    const text = await response.text();
    document.getElementById('changelog-content').innerHTML = marked.parse(text);
  } catch (error) {
    console.error('Error fetching changelog:', error);
  }
}

document.addEventListener('DOMContentLoaded', () => {
  if (document.getElementById('readme-content')) {
    fetchReadme();
  }
  if (document.getElementById('changelog-content')) {
    fetchChangelog();
  }
});