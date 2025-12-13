window.onload = function () {
  const footer = document.querySelector("footer");

  const year = 2024;
  const DATE = new Date();
  const FULLYEAR = DATE.getFullYear();
  let years = "";

  Number(year) == FULLYEAR ? (years = year) : (years = year + "-" + FULLYEAR);

  footer.innerHTML = "Copyright &copy; " + years + " by ZHENG Robert";
};
