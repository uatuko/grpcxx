#!/usr/bin/env node

const readline = require('readline');

const reader = readline.createInterface({
	input: process.stdin,
});

async function main() {
	const results  = {};
	let   output   = '';
	let   scenario = '';

	for await (const line of reader) {
		if (scenario === '' && !line.startsWith('[bench] :begin - ')) {
			continue;
		}

		if (scenario === '') {
			scenario = line.substr('[bench] :begin - '.length);
			continue;
		}

		if (line === `[bench] :end - ${scenario}`) {
			results[scenario] = {
				...results[scenario],
				output,
			};

			output   = '';
			scenario = '';
			continue;
		}

		output += `${line}\n`;

		if (line.startsWith('finished in ')) {
			const regex   = /^.*, (.*) req\/s,.*$/;
			const matches = line.match(regex);

			if (matches.length == 2) {
				results[scenario] = {
					...results[scenario],
					result : Math.round(matches[1]),
				};
			};
		}
	}

	let row = '| [TODO] |';
	let md  = '<details>\n' +
			'<summary>[TODO]</summary>' +
			'\n\n';

	for (const [scenario, result] of Object.entries(results)) {
		md += `### ${scenario}\n` +
			'```\n' +
			`${result.output}` +
			'```\n\n';

		row += ` ${Math.round(result.result / 1000)}k |`
	}

	md += '\n' +
		'</details>\n';

	console.log(md);
	console.log(row);
}

main();
