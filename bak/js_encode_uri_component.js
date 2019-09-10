function fixedEncodeURIComponent(str) {
  return encodeURIComponent(str).replace(/[!'()*]/g, function(c) {
    return '%' + c.charCodeAt(0).toString(16);
  });
}

console.log(encodeURIComponent('[!\'()*]'))
console.log(fixedEncodeURIComponent('[!\'()*]'))

const subDelims = '!$&\\()*+,;=';
console.log(encodeURIComponent(subDelims))
console.log(fixedEncodeURIComponent(subDelims))

const unreserved = 'aAzZ09-._~';
console.log(encodeURIComponent(unreserved))
console.log(fixedEncodeURIComponent(unreserved))
