#!/usr/bin/env node

const { env, stdin } = process;

const readStdin = () => {
  return new Promise((resolve) => {
    let body = '';
    stdin.setEncoding('utf8');
    stdin.on('data', chunk => body += chunk);
    stdin.on('end', () => resolve(body));
  });
};

(async () => {
  const method = env.REQUEST_METHOD || "GET";
  const query = env.QUERY_STRING || "";
  const contentType = env.CONTENT_TYPE || "";
  const contentLength = parseInt(env.CONTENT_LENGTH || "0", 10);

  let body = '';
  if (method === "POST" && contentLength > 0) {
    body = await readStdin();
  }

  console.log("Content-Type: text/html\n");

  console.log(`<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>JavaScript CGI Full Page</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 40px; background: #f9f9f9; color: #333; }
    h1 { color: #4CAF50; }
    pre { background: #eee; padding: 10px; border-radius: 5px; overflow: auto; }
    section { margin-top: 30px; }
    code { font-weight: bold; }
  </style>
</head>
<body>
  <h1>Welcome to JavaScript CGI</h1>

  <section>
    <h2>Request Info</h2>
    <ul>
      <li><strong>Method:</strong> ${method}</li>
      <li><strong>Query String:</strong> ${query}</li>
      <li><strong>Content-Type:</strong> ${contentType}</li>
      <li><strong>Content-Length:</strong> ${contentLength}</li>
    </ul>
  </section>

  <section>
    <h2>POST Body</h2>
    <pre>${body || "(no POST data)"}</pre>
  </section>

  <section>
    <h2>Environment Variables</h2>
    <pre>${Object.entries(env).map(([k, v]) => `${k}=${v}`).join('\n')}</pre>
  </section>

  <section>
    <h2>Try Sending a POST</h2>
    <form method="POST" action="${env.SCRIPT_NAME || ''}">
      <label>Message: <input type="text" name="message" /></label>
      <input type="submit" value="Send" />
    </form>
  </section>

</body>
</html>`);
})();
