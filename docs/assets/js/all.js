window.onload = function () {
  const footer = document.querySelector("#footer-center");

  const year = 2024;
  const DATE = new Date();
  const FULLYEAR = DATE.getFullYear();
  let years = "";

  Number(year) == FULLYEAR ? (years = year) : (years = year + "-" + FULLYEAR);

  footer.innerHTML =
    "<p>&copy; " +
    years +
    " by ZHENG Robert</p><p class='txt-small'>All rights reserved.</p>";
};
